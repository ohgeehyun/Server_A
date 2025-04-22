#include "pch.h"
#include "ClientPacketHandler.h"
#include "ServerPacketHandler.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include "ServerSession.h"
#include "ServerSessionManager.h"
#include "Player.h"
#include "RoomManager.h"
#include "DataManager.h"
#include "RedisConnection.h"
#include "RedisUtils.h"
#include "DataManager.h"
#include <jwt-cpp/jwt.h>
#include <httplib/httplib.h>


PacketHandlerFunc GPacketHandler[UINT16_MAX];

bool Handle_C_ENTER_GAME(PacketSessionRef& session, Protocol::C_ENTER_GAME& pkt)
{
    return true;
}

bool Handle_C_CREATE_ROOM(PacketSessionRef& session, Protocol::C_CREATE_ROOM& pkt)
{
    GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

    if (pkt.roomname() == "")
        return false;

    // RoomServer에게 방 생성 요청
    ServerProtocol::C_CREATE_ROOM packet;
    packet.set_roomname(pkt.roomname());
    packet.set_roompwd(pkt.roompwd());
    packet.set_rootuser(gameSession->GetNickName());
    packet.set_sessionid(gameSession->GetSessionId());

    ServerSessionRef RoomServerSession = GServerSessionManager->Pop_Session();

    auto Packet = ServerPacketHandler::MakeSendBuffer(packet);
    RoomServerSession->Send(Packet);

    return true;
}

bool Handle_C_ROOM_LIST(PacketSessionRef& session, Protocol::C_ROOM_LIST& pkt)
{
    return false;
}

bool Handle_C_LEAVE_GAME(PacketSessionRef& session, Protocol::C_LEAVE_GAME& pkt)
{
    return false;
}

bool Handle_C_MOVE(PacketSessionRef& session, Protocol::C_MOVE& pkt)
{
    return true;
}

bool Handle_C_SKILL(PacketSessionRef& session, Protocol::C_SKILL& pkt)
{
    return true;
}

bool Handle_C_VERIFY(PacketSessionRef& session, Protocol::C_VERIFY& pkt)
{
    JwtUtils jwt;
    jwt.JwtVerify(pkt.jwt(), "verify login token ");

    if (jwt.GetVerifyStat())
    {
       nlohmann::json payload = jwt.GetPayload_Json();
       string user_id = payload["user_id"];
       string nickname = payload["nickname"];      

       // PacketSessionRef를 GameSession으로 캐스팅
       GameSessionRef gameSession = dynamic_pointer_cast<GameSession>(session);
       gameSession->SetUserId(user_id);
       gameSession->SetNickName(nickname);
       gameSession->SetJwtToken(pkt.jwt());
       gameSession->SetIsJwtVerify(jwt.GetVerifyStat());
       gameSession = nullptr;

       //jwt 검증완료 후 검증이 완료 된 후 활성화된 유저 수 redis에 업데이트
       const char* query = "HSET active_user %s %s";
       RedisUtils::RAsyncCommand(GRedisConnection->GetContext(), query, user_id.c_str(), nickname.c_str());

       Protocol::S_VERIFY packet;
       packet.set_result(true);
       packet.set_userid(user_id);
       packet.set_nickname(nickname);
       auto PacketBuffer = ClientPacketHandler::MakeSendBuffer(packet);
       session->Send(PacketBuffer);

    }

    return true;
}

bool Handle_C_MESSAGE(PacketSessionRef& session, Protocol::C_MESSAGE& pkt)
{
    return false;
}


