#include "pch.h"
#include "ClientPacketHandler.h"
#include "ServerPacketHandler.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include "RoomSession.h"
#include "RoomSessionManager.h"
#include "Player.h"
#include "RoomManager.h"
#include "DataManager.h"
#include "RedisManager.h"
#include <jwt-cpp/jwt.h>
#include <httplib/httplib.h>


PacketHandlerFunc GPacketHandler[UINT16_MAX];

bool Handle_C_CREATE_ROOM(GameSessionRef& session, Protocol::C_CREATE_ROOM& pkt)
{
    if (pkt.roomname() == "")
        return false;

    // RoomServer에게 방 생성 요청
    ServerProtocol::C_CREATE_ROOM packet;
    packet.set_roomname(pkt.roomname());
    packet.set_roompwd(pkt.roompwd());
    packet.set_rootuser(session->GetNickName());
    packet.set_sessionid(session->GetSessionId());

    RoomSessionRef RoomServerSession = GRoomSessionManager->Pop_Session();

    auto Packet = ServerPacketHandler::MakeSendBuffer(packet);
    RoomServerSession->Send(Packet);

    return true;
}
bool Handle_C_ENTER_GAME(GameSessionRef& session, Protocol::C_ENTER_GAME& pkt)
{
    if (pkt.rommid() <= 0)
        return false;

    PacketSessionRef RoomServerSession = RoomManager::GetInstance().FindToSession(pkt.rommid());

    if (RoomServerSession == nullptr)
    {
        //존재하지 않는 방 입장 또는 모종의 버그로 UserServer와 RoomServer 연결이 되지않은 서버 존재
        //Redis에서 방정보 검색 후 있으면 연결 후 제공 없다면 클라이언트에게 에러 전송
    }
    else
    {
        //바로 해당 RoomServerSession에 입장 패킷전송
        ServerProtocol::C_ENTER_GAME packet;
        packet.set_rommid(pkt.rommid());
        packet.set_roomname(pkt.roomname());
        packet.set_userid(session->GetUserId());
        packet.set_usernickname(session->GetNickName());
        auto Packet = ServerPacketHandler::MakeSendBuffer(packet);
        RoomServerSession->Send(Packet);
    }

    return true;
}

bool Handle_C_ROOM_LIST(GameSessionRef& session, Protocol::C_ROOM_LIST& pkt)
{
    return false;
}

bool Handle_C_LEAVE_GAME(GameSessionRef& session, Protocol::C_LEAVE_GAME& pkt)
{
    if (pkt.exitflag() == false)
        return false;

    PlayerRef player = session->GetPlayer();
    RoomRef room = player->GetRoom();
    PacketSessionRef RoomServerSession = RoomManager::GetInstance().FindToSession(room->GetRoomId());
    if (player == nullptr || room == nullptr || RoomServerSession == nullptr)
        return false;
  
    ServerProtocol::C_LEAVE_GAME leavepacket;
    leavepacket.set_exitflag(pkt.exitflag());;
    leavepacket.set_objectid(player->GetObjectId());
    leavepacket.set_roomid(room->GetRoomId());
    auto leavePacket = ServerPacketHandler::MakeSendBuffer(leavepacket);
    RoomServerSession->Send(leavePacket);

    return true;
}

bool Handle_C_MOVE(GameSessionRef& session, Protocol::C_MOVE& pkt)
{
    //바로 if문에서 player정보를 체크해도 되지만 멀티스레드환경에서 안전 하지 못 하다.
    PlayerRef player = session->GetPlayer();
    RoomRef room = player->GetRoom();

    if (player == nullptr || room == nullptr)
        return false;

    PacketSessionRef RoomServerSession = RoomManager::GetInstance().FindToSession(room->GetRoomId());
    if (RoomServerSession == nullptr)
        return false;

    //오브젝트 id, roomid ,posinfo를 패킷에 담아 룸 서버에게 전송
    ServerProtocol::C_MOVE packet;
    packet.mutable_posinfo()->CopyFrom(pkt.posinfo());
    packet.set_objectid(player->GetObjectId());
    packet.set_roomid(room->GetRoomId());
    auto Packet = ServerPacketHandler::MakeSendBuffer(packet);
    RoomServerSession->Send(Packet);

    return true;
}

bool Handle_C_SKILL(GameSessionRef& session, Protocol::C_SKILL& pkt)
{
    PlayerRef player = session->GetPlayer();
    RoomRef room = player->GetRoom();
    PacketSessionRef RoomServerSession = RoomManager::GetInstance().FindToSession(room->GetRoomId());
    if (player == nullptr || room == nullptr|| RoomServerSession == nullptr)
        return false;

    ServerProtocol::C_SKILL packet;
    packet.set_objectid(player->GetObjectId());
    packet.set_roomid(room->GetRoomId());
    packet.mutable_info()->CopyFrom(pkt.info());
    auto Packet = ServerPacketHandler::MakeSendBuffer(packet);
    RoomServerSession->Send(Packet);

    return true;
}

bool Handle_C_VERIFY(GameSessionRef& session, Protocol::C_VERIFY& pkt)
{
    JwtUtils jwt;
    jwt.JwtVerify(pkt.jwt(), "verify login token ");

    if (!jwt.GetVerifyStat())
        return false;
    
    nlohmann::json payload = jwt.GetPayload_Json();
    string user_id = payload["user_id"];
    string nickname = payload["nickname"];   

    //reconnect인지 판단
    bool reConnect = payload.value("reConnect", false);

    if (reConnect)
    {
        GGameSessionManager->TransferSessionData(user_id,session,pkt.jwt());
        return true;
    }
    else
    {
        session->SetUserId(user_id);
        session->SetNickName(nickname);
        session->SetJwtToken(pkt.jwt());
        session->SetIsJwtVerify(jwt.GetVerifyStat());
        session->InitPlayer();
        GGameSessionManager->Add_UserIdToSession(session, user_id);

        nlohmann::json user_info_json = JsonUtils::createJson(
            std::make_pair("user_id", user_id),
            std::make_pair("nickname", nickname),
            std::make_pair("session_id", session->GetSessionId())
        );

        string redis_key = user_id;
        string redis_value = user_info_json.dump(); // JSON → string 직렬화

        const char* query = "SET user_info:%s %s";
        RedisManager::GetInstance().RAsyncCommand(query, redis_key.c_str(), redis_value.c_str());

        Protocol::S_VERIFY packet;
        packet.set_result(true);
        packet.set_userid(user_id);
        packet.set_nickname(nickname);
        auto Packet = ClientPacketHandler::MakeSendBuffer(packet);
        session->Send(Packet);
        return true;
    }
}

bool Handle_C_MESSAGE(GameSessionRef& session, Protocol::C_MESSAGE& pkt)
{
    PlayerRef player= session->GetPlayer();
    PacketSessionRef RoomServerSession = RoomManager::GetInstance().FindToSession(pkt.rommid());
    if (RoomServerSession == nullptr || player == nullptr)
        return false;

    ServerProtocol::C_MESSAGE packet;
    packet.set_rommid(pkt.rommid());
    packet.set_message(pkt.message());
    packet.set_nickname(pkt.nickname());
    packet.set_objectid(player->GetObjectId());
    auto Packet = ServerPacketHandler::MakeSendBuffer(packet);
    session->Send(Packet);
   
    return true;
}


