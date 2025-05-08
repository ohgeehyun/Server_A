#include "pch.h"
#include "RoomPacketHandler.h"
#include "Room.h"
#include "Player.h"
#include "RoomManager.h"

ServerPacketHandlerFunc GServerPacketHandler[UINT16_MAX];


bool Handle_C_ENTER_GAME(PacketSessionRef& session, ServerProtocol::C_ENTER_GAME& pkt)
{
     RoomRef room = GRoomManager->Find(pkt.rommid());

     if (room == nullptr)
         return false;

     //해당 방에 유저가 있는지 검사 없다면 첫 입장(플레이어 객체 생성) 있다면 재접속 또는 Respawn상태
     PlayerRef isPlaying = GRoomManager->GetUserInRoom(pkt.rommid(), pkt.userid());

     if (isPlaying)
     {
         room->DoAsync(&Room::EnterGame, static_pointer_cast<GameObject>(isPlaying));
     }
     else
     {
         PlayerRef player = room->GetObjManager().Add<Player>();
         player->initPlayer(pkt.userid());
         player->SetSession(session);
         player->SetUserNickName(pkt.usernickname());
         room->DoAsync(&Room::EnterGame,static_pointer_cast<GameObject>(player));
     }

     return true;;
}
bool Handle_C_LEAVE_GAME(PacketSessionRef& session, ServerProtocol::C_LEAVE_GAME& pkt)
{
    if (pkt.exitflag() == false)
        return false;

    RoomRef room = GRoomManager->Find(pkt.roomid());
    PlayerRef player = room->GetPlayer(pkt.objectid());

    if (room == nullptr || player == nullptr)
        return false;

    room->DoAsync(&Room::LeaveGame, player->GetObjectId());

    room->DoAsync(&Room::ExitGameEventSend, std::move(player));

    return true;
}
bool Handle_C_MOVE(PacketSessionRef& session, ServerProtocol::C_MOVE& pkt)
{
    //바로 if문에서 player정보를 체크해도 되지만 멀티스레드환경에서 안전 하지 못 하다.
    RoomRef room = GRoomManager->Find(pkt.roomid());
    PlayerRef player = room->GetPlayer(pkt.objectid());

    if (room == nullptr || player == nullptr)
        return false;

    room->DoAsync(std::bind(&Room::HandleMove, room, player, pkt));

    cout << "C_MOVE : " << pkt.posinfo().posx() << "," << pkt.posinfo().posy() << endl;
    
    return true;
}
bool Handle_C_SKILL(PacketSessionRef& session, ServerProtocol::C_SKILL& pkt)
{

    RoomRef room = GRoomManager->Find(pkt.roomid());
    PlayerRef player = room->GetPlayer(pkt.objectid());
    if (room == nullptr || player == nullptr)
        return false;

    room->DoAsync(&Room::HandleSkill, player, pkt);
    return true;
}

bool Handle_C_CREATE_ROOM(PacketSessionRef& session, ServerProtocol::C_CREATE_ROOM& pkt)
{
    //방 이름 검사
    if (pkt.roomname() == "")
        return false;

    // Redis 비동기 요청 + 콜백 정의
    GRoomManager->DoAsync(std::bind(&RoomManager::RoomidToRedis_CreateRoom, GRoomManager, pkt, session),true);
 
    return true;
}

bool Handle_C_MESSAGE(PacketSessionRef& session, ServerProtocol::C_MESSAGE& pkt)
{
    RoomRef room = GRoomManager->Find(pkt.rommid());
    PlayerRef player = room->GetPlayer(pkt.objectid());

    if (player == nullptr || room == nullptr)
        return false;

    if (room->GetRoomId() != pkt.rommid())
        return false;

    ServerProtocol::S_MESSAGE message;
    message.set_rommid(pkt.rommid());
    message.set_nickname(pkt.nickname());
    message.set_message(pkt.message());
    message.set_objectid(pkt.objectid());
    auto messageBuffer = RoomPacketHandler::MakeSendBuffer(message);

    room->DoAsync(std::bind(&Room::BroadcastExcept, room, messageBuffer, player));

    auto now = std::chrono::system_clock::now();
    std::time_t time_now = std::chrono::system_clock::to_time_t(now);
    std::tm tm_now;
    localtime_s(&tm_now, &time_now);

    std::ostringstream oss;
    oss << std::put_time(&tm_now, "%Y-%m-%d %H:%M:%S");
    std::string timestamp = oss.str();  // 시간 포맷팅

    const char* query = "XADD room_chat:%d * user_id %s nickname %s message %s timestamp %s";
    RedisManager::GetInstance().RAsyncCommand(query, pkt.rommid(), player->GetUserId().c_str(), pkt.nickname().c_str(), pkt.message().c_str(), timestamp.c_str());
    
    return true;
}

