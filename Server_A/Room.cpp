#include "pch.h"
#include "Room.h"
#include "GameObject.h"
#include "Player.h"
#include "Monster.h"
#include "Arrow.h"
#include "ClientPacketHandler.h"
#include "ServerPacketHandler.h"
#include "GameSession.h"
#include "ObjectManager.h"
#include "DataManager.h"
#include "MagicSkill.h"
#include "RoomManager.h"
#include "RedisConnection.h"
#include "RoomSession.h"
#include <httplib/httplib.h> 

Room::Room() 
{
    
}

Room::~Room()
{
    RoomManager::GetInstance().Remove(_roomId);
    cout << _roomId << " 번 방 소멸자 호출 완료." << endl;
}

void Room::RoomBreak(PlayerRef player)
{
    Protocol::S_EXIT_GAME packet;
    packet.set_exitflag(true);
    auto exitPacketBuffer = ClientPacketHandler::MakeSendBuffer(packet);
    DoAsync(&Room::BroadcastExcept, std::move(exitPacketBuffer), player->GetObjectId());

    for (auto& player : _players)
    {
        player.second->SetRoom(nullptr);
    }
    _players.clear();

    for (auto& player : _userIdToPlayers)
    {
        player.second->SetRoom(nullptr);
    }
    _userIdToPlayers.clear();
}

PlayerRef Room::GetPlayer(string userid)
{
    auto it = _userIdToPlayers.find(userid);

    if (it == _userIdToPlayers.end())
        return nullptr;

    return it->second;
}

PlayerRef Room::GetPlayer(int32 objectid)
{
    auto it = _players.find(objectid);

    if (it == _players.end())
        return nullptr;

    return it->second;
}

void Room::EnterGame(int32 roomId, GameObjectRef object, GameSessionRef ClientSession)
{
    if (object == nullptr)
        return;
    if (ClientSession == nullptr)
        return;
    
    //현재 유저서버에서 만들어진 적이 없는 방이거나 저장된 룸 아이디에 맞는 세션이없다면 서버로직중 예외 발생했거나 말도안되는 룸id를 패킷으로 받은 것 
    PacketSessionRef RoomServerSession = RoomManager::GetInstance().FindToSession(roomId);
    if (RoomServerSession)
        return;

    ServerProtocol::C_ENTER_GAME pkt;
    pkt.set_rommid(roomId);
    auto Packet = ServerPacketHandler::MakeSendBuffer(pkt);
    RoomServerSession->Send(Packet);

    RoomManager::GetInstance().Remove(_roomId);
}

void Room::EnterRoom_Player(int32 objectid, PlayerRef player)
{
    _players[objectid] = player;
    _userIdToPlayers[player->GetSession()->GetUserId()] = player;
    player->SetObjectId(objectid);
    player->SetRoom(GetRoomSharedPtr());
}

void Room::LeaveGame(ServerProtocol::S_LEAVE_GAME& pkt)
{
    auto it = _players.find(pkt.objectid());
    PlayerRef player = it->second;
    if (player == nullptr)
        return;
    //플레이어 와 Room 의존 끊기
    player->SetRoom(nullptr);

    //플레이어 목록에서 삭제
    _players.erase(pkt.objectid());
    _userIdToPlayers.erase(player->GetSession()->GetUserId());

     DoAsync(&Room::LeaveGame_Send, std::move(player),pkt.objectid());
}

void Room::LeaveGame_Send(PlayerRef player, int32 objectid)
{
    if (player == nullptr)
        return;
    
    Protocol::S_LEAVE_GAME packet;
    packet.set_exit(true);
    auto Packet = ClientPacketHandler::MakeSendBuffer(packet);
    player->GetSession()->Send(Packet);

    //서버에 접속인원에게 퇴장 패킷 생성
    Protocol::S_DESPAWN despawnPacket;
    despawnPacket.add_objectids(objectid);
    despawnPacket.set_roomid(GetRoomId());
    auto despawnPacketBuffer = ClientPacketHandler::MakeSendBuffer(despawnPacket);

    // 다른 플레이어에게만 패킷 전송 
    for (const auto& pair : _players) {
            pair.second->GetSession()->Send(despawnPacketBuffer);
    }

}

void Room::Broadcast(SendBufferRef buffer)
{
    for (const auto& playerPair : _players)
    {
        playerPair.second->GetSession()->Send(buffer);
        cout << "Broadcast 전송된 플레이어 id : " << playerPair.second->GetSession()->GetUserId()<< endl;
    }
}

void Room::BroadcastExcept(SendBufferRef buffer , int32 objectid)
{
    for (const auto pair : _players)
    {
        if (pair.first != objectid)
        {
            pair.second->GetSession()->Send(buffer);
        }
    }
}

void Room::Send(const string& userid, SendBufferRef buffer)
{
    auto target = _userIdToPlayers.find(userid);

    if (target == _userIdToPlayers.end())
        return; 

    target->second->GetSession()->Send(buffer);
}

void Room::HandleMoveEvent(int32 objectid, SendBufferRef buffer)
{
    for (const auto& pair : _players)
    {
        if (pair.first != objectid)
        {
            pair.second->GetSession()->Send(buffer);
            cout << "Object Move Event !" << endl;
        }
    }
}

void Room::ExitGameEventSend(PlayerRef player)
{
    //게임 방에서 완전히 나가기가 완료되었다고 클라이언트에게 패킷을 전송해주자
    Protocol::S_EXIT_GAME packet;
    packet.set_exitflag(true);
    auto exitPacketBuffer = ClientPacketHandler::MakeSendBuffer(packet);
    player->GetSession()->Send(exitPacketBuffer);

    //room을 만든 user가 방에서 나감
    if (player->GetSession()->GetNickName() == GetRootUser())
        DoAsync(&Room::RoomBreak, std::move(player));
}

void Room::DeSawnPacketSend(ServerProtocol::S_DESPAWN& pkt)
{
    Protocol::S_DESPAWN packet;
    packet.mutable_objectids()->CopyFrom(pkt.objectids());
    packet.set_roomid(GetRoomId());
    auto Packet = ClientPacketHandler::MakeSendBuffer(packet);

    for (int32 i = 0; i < pkt.objectids().size(); i++)
    {
        DoAsync(&Room::BroadcastExcept, std::move(Packet), pkt.objectids(i));
    }
}
