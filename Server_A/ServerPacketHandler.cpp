#include "pch.h"
#include "ServerPacketHandler.h"
#include "ClientPacketHandler.h"
#include "GameSessionManager.h"
#include "Room.h"
#include "RoomManager.h"

ServerPacketHandlerFunc GServerPacketHandler[UINT16_MAX];

bool Handle_S_ENTER_GAME(PacketSessionRef& session, ServerProtocol::S_ENTER_GAME& pkt)
{
    // ENTER_GAME은 플레이어 전용 이벤트 몬스터 또는 오브젝트라면 ENTER_GAME이 아닌spawn으로 가기 때문
    if (pkt.roomid() <= 0)
        return false;

    GameSessionRef ClientSession = GGameSessionManager->Find_UserIdToSession(pkt.userid());
    PlayerRef player = ClientSession->GetPlayer();
    RoomRef room = RoomManager::GetInstance().Find(pkt.roomid());

    if (session == nullptr || room == nullptr || player == nullptr)
        return false;

    //Room에 플레이어 정보 담을 것 즉시 실행되어야하는 정보기 때문에 비동기 처리x
    room->EnterRoom_Player(pkt.player().objectid(), player);

    Protocol::S_ENTER_GAME packet;
    *packet.mutable_player() = pkt.player();
    packet.set_roomid(pkt.roomid());
    packet.set_roomname(pkt.roomname());
    SendBufferRef Packet = ClientPacketHandler::MakeSendBuffer(packet);

    room->DoAsync(&Room::Send, pkt.userid(), std::move(Packet)); 

    return true;
}

bool Handle_S_LEAVE_GAME(PacketSessionRef& session, ServerProtocol::S_LEAVE_GAME& pkt)
{
    if (pkt.exit() == false)
        return false;
    
   RoomRef room = RoomManager::GetInstance().Find(pkt.roomid());
   
   if (room == nullptr)
       return false;

   room->DoAsync(&Room::LeaveGame,pkt);

    return true;
}

bool Handle_S_EXIT_GAME(PacketSessionRef& session, ServerProtocol::S_EXIT_GAME& pkt)
{
    if (pkt.exitflag() == false)
        return false;

    RoomRef room = RoomManager::GetInstance().Find(pkt.roomid());
    PlayerRef player = room->GetPlayer(pkt.objectid());
    if (room == nullptr)
        return false;

    //User서버에서도 결국 Room의 플레이어 정리를 해주어야 함.
    room->ExitGameEventSend(player);
    return true;
}

bool Handle_S_SPAWN(PacketSessionRef& session, ServerProtocol::S_SPAWN& pkt)
{
    if (pkt.roomid() <= 0)
        return false;

    RoomRef room = RoomManager::GetInstance().Find(pkt.roomid());

    if (room == nullptr)
        return false;

    if (pkt.userid().empty())
    {
        //특정 대상이아니라 방 전체 인원에게 해당 오브젝트 스폰을 전송 
        //그럼 해당방에 players에서 players의 session에 전송을해줘야함 일단 EnterGame에서 player정보를 만들어야할듯
        Protocol::S_SPAWN packet;
        *packet.mutable_objects() = pkt.objects();
        SendBufferRef Packet = ClientPacketHandler::MakeSendBuffer(packet);
        room->DoAsync(&Room::Broadcast,std::move(Packet));
    }
    else
    {
        Protocol::S_SPAWN packet;
        *packet.mutable_objects() = pkt.objects();
        SendBufferRef Packet = ClientPacketHandler::MakeSendBuffer(packet);
        room->DoAsync(&Room::Send, pkt.userid(), std::move(Packet));
    }

    return true;
}

bool Handle_S_DESPAWN(PacketSessionRef& session, ServerProtocol::S_DESPAWN& pkt)
{
    if (pkt.objectids().size() == 0 || pkt.roomid() <= 0)
        return false;

    RoomRef room =  RoomManager::GetInstance().Find(pkt.roomid());
    room->DoAsync(&Room::DeSawnPacketSend, pkt);

    return false;
}

bool Handle_S_MOVE(PacketSessionRef& session, ServerProtocol::S_MOVE& pkt)
{
    RoomRef room = RoomManager::GetInstance().Find(pkt.roomid());
    if (room == nullptr)
        return false;

    //다른 플레이어에게도 이동 전달
    Protocol::S_MOVE packet;
    packet.set_objectid(pkt.objectid());
    packet.mutable_posinfo()->CopyFrom(pkt.posinfo());
    auto Packet = ClientPacketHandler::MakeSendBuffer(packet);
    room->DoAsync(&Room::HandleMoveEvent, packet.objectid(), std::move(Packet));
    
    return true;
}

bool Handle_S_CREATE_ROOM(PacketSessionRef& session, ServerProtocol::S_CREATE_ROOM& pkt)
{
    //UserServer도 room정보를 가지고있을까말까?  고민되넹
    if (pkt.result() != false)
    {
        RoomManager::GetInstance().Add(pkt);
        RoomManager::GetInstance().Add_RoomIdToServerSession(pkt.roomid(),session);
    }
        

    Protocol::S_CREATE_ROOM packet;
    packet.set_result(pkt.result());
    packet.set_roomid(pkt.roomid());
    SendBufferRef Packet = ClientPacketHandler::MakeSendBuffer(packet);

    //룸 생성 요청을 한 클라이언트의 세션 
    GGameSessionManager->Find(pkt.sessionid())->Send(Packet);;

    return true;
}

bool Handle_S_SKILL(PacketSessionRef& session, ServerProtocol::S_SKILL& pkt)
{
    RoomRef room = RoomManager::GetInstance().Find(pkt.roomid());
    if (room == nullptr)
        return false;

    Protocol::S_SKILL packet;
    packet.set_objectid(pkt.objectid());
    packet.mutable_info()->CopyFrom(pkt.info());
    SendBufferRef Packet = ClientPacketHandler::MakeSendBuffer(packet);
    room->DoAsync(&Room::Broadcast, std::move(Packet));
    return true;;
}

bool Handle_S_MESSAGE(PacketSessionRef& session, ServerProtocol::S_MESSAGE& pkt)
{

    RoomRef room = RoomManager::GetInstance().Find(pkt.rommid());

    if (room == nullptr)
        return false;

    Protocol::S_MESSAGE packet;
    packet.set_rommid(pkt.rommid());
    packet.set_nickname(pkt.nickname());
    packet.set_message(pkt.message());
    auto Packet = ClientPacketHandler::MakeSendBuffer(packet);
    room->DoAsync(std::bind(&Room::BroadcastExcept, room, Packet, pkt.objectid()));

    return true;
}

bool Handle_S_CHANGEHP(PacketSessionRef& session, ServerProtocol::S_CHANGEHP& pkt)
{
    RoomRef room = RoomManager::GetInstance().Find(pkt.roomid());
    if (room == nullptr)
        return false;

    Protocol::S_CHANGEHP packet;
    packet.set_objectid(pkt.objectid());
    packet.set_hp(pkt.hp());
    SendBufferRef Packet = ClientPacketHandler::MakeSendBuffer(packet);
    room->DoAsync(&Room::Broadcast, std::move(Packet));
    
    return true;
}

bool Handle_S_DIE(PacketSessionRef& session, ServerProtocol::S_DIE& pkt)
{
    RoomRef room = RoomManager::GetInstance().Find(pkt.roomid());
    if (room == nullptr)
        return false;

    Protocol::S_DIE packet;
    packet.set_objectid(pkt.objectid());
    packet.set_attackerid(pkt.attackerid());
    SendBufferRef Packet = ClientPacketHandler::MakeSendBuffer(packet);
    room->DoAsync(&Room::Broadcast, std::move(Packet));

    return true;
}
