#include "pch.h"
#include "ClientPacketHandler.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include "Player.h"
#include "RoomManager.h"
#include "DataManager.h"
#include "RedisConnection.h"
#include "RedisUtils.h"
#include "DataManager.h"
#include <jwt-cpp/jwt.h>
#include <httplib/httplib.h>


PacketHandlerFunc GPacketHandler[UINT16_MAX];

bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len)
{
    RecvPacketHeader* header = reinterpret_cast<RecvPacketHeader*>(buffer);
    // TODO : Log
    return true;
}

bool Handle_C_ENTER_GAME(PacketSessionRef& session, Protocol::C_ENTER_GAME& pkt)
{

    RoomRef room = RoomManager::GetInstance().Find(pkt.rommid());

    if (room->GetPlayerCount() >= MAX_USER_COUNT)
    {
        //방 정원 초과 S_ENTER_GAME 패킷발송 방ID_0은 room입장실패
        Protocol::S_ENTER_GAME packet;
        packet.set_roomid (0);
        auto PacketBuffer = ClientPacketHandler::MakeSendBuffer(packet);
        session->Send(PacketBuffer);
    }

    //session 캐스팅
    GameSessionRef gameSession = static_pointer_cast<GameSession>(session);
    //player를 초기화
    gameSession->InitPlayer();
     
    cout << "플레이어 입장" << endl;
    //방 입장 gameSession->GetPlayer()가 참조값을 반환
    room->EnterGame(std::static_pointer_cast<GameObject>(gameSession->GetPlayer()));
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

bool Handle_C_LEAVE_GAME(PacketSessionRef& session, Protocol::C_LEAVE_GAME& pkt)
{
    GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

    if (pkt.exitflag() != true)
        return false;

    if (gameSession->GetPlayer() == nullptr || gameSession->GetPlayer()->GetRoom() == nullptr)
        return false;

    RoomRef room = gameSession->GetPlayer()->GetRoom();

    room->DoAsync(&Room::LeaveGame,gameSession->GetPlayer()->GetObjectId());

    room->DoAsync(&Room::ExitGameEventSend,gameSession->GetPlayer());

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

    auto now = std::chrono::system_clock::now();
    std::time_t time_now = std::chrono::system_clock::to_time_t(now);
    std::tm tm_now;
    localtime_s(&tm_now,&time_now);

    std::ostringstream oss;
    oss << std::put_time(&tm_now, "%Y-%m-%d %H:%M:%S");
    std::string timestamp = oss.str();  // 시간 포맷팅

    const char* query = "XADD room_chat:%d * user_id %s nickname %s message %s timestamp %s";
    RedisUtils::RAsyncCommand(GRedisConnection->GetContext(), query, pkt.rommid(), gameSession->GetUserId().c_str(), pkt.nickname().c_str(), pkt.message().c_str(), timestamp.c_str());

    return false;
}


