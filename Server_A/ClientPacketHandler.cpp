#include "pch.h"
#include "ClientPacketHandler.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include "Player.h"
#include "RoomManager.h"
#include "DataManager.h"
#include "RedisConnection.h"
#include "RedisUtils.h"
#include <jwt-cpp/jwt.h>


PacketHandlerFunc GPacketHandler[UINT16_MAX];

bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len)
{
    RecvPacketHeader* header = reinterpret_cast<RecvPacketHeader*>(buffer);
    // TODO : Log
    return true;
}

bool Handle_C_ENTER_GAME(PacketSessionRef& session, Protocol::C_ENTER_GAME& pkt)
{
    //session 캐스팅
    GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

    //player를 초기화
    gameSession->InitPlayer();

    RoomRef room = RoomManager::GetInstance().Find(pkt.rommid());
    cout << "플레이어 입장" << endl;
    //방 입장 gameSession->GetPlayer()가 참조값을 반환
    room->EnterGame(std::static_pointer_cast<GameObject>(gameSession->GetPlayer()));
    //room->DoAsync(&Room::EnterGame, std::static_pointer_cast<GameObject>(gameSession->GetPlayer()));
    return true;
}

bool Handle_C_CREATE_ROOM(PacketSessionRef& session, Protocol::C_CREATE_ROOM& pkt)
{
    GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

    if (pkt.roomname() == "")
        return false;
    //룸생성 호출 
    RoomRef room = RoomManager::GetInstance().Add(1, pkt.roomname(), pkt.roompwd(),gameSession->GetNickName());
    
    Protocol::S_CREATE_ROOM resultPacket;
    if (room != nullptr)
    {
        resultPacket.set_result(true);
        resultPacket.set_roomid(room->GetRoomId());
    }
    else 
    {
        resultPacket.set_result(false);
    }
    auto resultPacketBuffer = ClientPacketHandler::MakeSendBuffer(resultPacket);
    session->Send(resultPacketBuffer);

    return true;
}

bool Handle_C_ROOM_LIST(PacketSessionRef& session, Protocol::C_ROOM_LIST& pkt)
{
    return false;
}

bool Handle_C_MOVE(PacketSessionRef& session, Protocol::C_MOVE& pkt)
{
    GameSessionRef gameSession = static_pointer_cast<GameSession>(session);
    
    cout << "C_MOVE : " << pkt.posinfo().posx() <<","<< pkt.posinfo().posy()<< endl;
    
    //바로 if문에서 player정보를 체크해도 되지만 멀티스레드환경에서 안전 하지 못 하다.
    PlayerRef player = gameSession->GetPlayer();
    if (player == nullptr)
        return false;

    RoomRef room = player->GetRoom();
    if (room == nullptr)
        return false;

    room->DoAsync(std::bind(&Room::HandleMove, room, player, pkt));
   
    return true;
}

bool Handle_C_SKILL(PacketSessionRef& session, Protocol::C_SKILL& pkt)
{
    GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

    PlayerRef player = gameSession->GetPlayer();
    if (player == nullptr)
        return false;

    RoomRef room = player->GetRoom();
    if (room == nullptr)
        return false;

    room->DoAsync(std::bind(&Room::HandleSkill, room, player, pkt));
    return true;
}

bool Handle_C_VERIFY(PacketSessionRef& session, Protocol::C_VERIFY& pkt)
{
    JwtUtils jwt;
    jwt.JwtVerify(pkt.jwt(), "verify logintoken ");

    if (jwt.GetVerifyStat())
    {
       nlohmann::json payload = jwt.GetPayload_Json();
       string user_id = payload["user_id"];
       string nickname = payload["nickname"];      

       // PacketSessionRef를 GameSession으로 캐스팅
       GameSessionRef gameSession = static_pointer_cast<GameSession>(session);
       gameSession->SetUserId(payload["user_id"]);
       gameSession->SetNickName(payload["nickname"]);
       gameSession = nullptr;

       std::weak_ptr<PacketSession> weakSession = session;

       redisAsyncCommand(GRedisConnection->GetContext(), [](redisAsyncContext* context, void* reply, void* privdata)
       {
           if (reply != nullptr)
               RedisUtils::replyResponseHandler(reply, "Redis active_user add : ");
           
           auto weakSession = static_cast<std::weak_ptr<PacketSession>*>(privdata);
          
           if (auto session = weakSession->lock()) // 유효한 shared_ptr 복원
           {
               Protocol::S_VERIFY packet;
               packet.set_result(true);
               auto PacketBuffer = ClientPacketHandler::MakeSendBuffer(packet);
               session->Send(PacketBuffer);
           }
       }, &weakSession, "HSET active_user %s %s", user_id.c_str(), nickname.c_str());
    }

    return true;
}

bool Handle_C_MESSAGE(PacketSessionRef& session, Protocol::C_MESSAGE& pkt)
{
    GameSessionRef gameSession = static_pointer_cast<GameSession>(session);


    PlayerRef player = gameSession->GetPlayer();
    if (player == nullptr)
        return false;

    RoomRef room = player->GetRoom();
    if (room == nullptr)
        return false;
   
    if (room->GetRoomId() != pkt.rommid())
        return false;

    Protocol::S_MESSAGE message;
    message.set_rommid(pkt.rommid());
    message.set_nickname(pkt.nickname());
    message.set_message(pkt.message());
    auto messageBuffer = ClientPacketHandler::MakeSendBuffer(message);

    room->DoAsync(std::bind(& Room::BroadcastExcept,room,messageBuffer, player));
    return false;
}


