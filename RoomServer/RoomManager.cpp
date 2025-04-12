#include "pch.h"
#include "Room.h"
#include "RoomManager.h"
#include "ServerProtocol.pb.h"
#include "RedisConnection.h"
#include "RoomPacketHandler.h"
#include "UserServerSession.h"
#include "RedisUtils.h"
#include "JsonUtils.h"
#include <nlohmann/json.hpp>

RoomRef RoomManager::Add(const ServerProtocol::C_CREATE_ROOM& pkt, int32 roomId, UserServerSessionRef session)
{
    RoomRef gameRoom = Make_Shared<Room>();

    WRITE_LOCK
    {
        gameRoom->SetRoomId(roomId);
        gameRoom->SetRoomName(pkt.roomname());
        gameRoom->SetRoomPwd(pkt.roompwd());
        gameRoom->SetRootUser(pkt.rootuser());

        if (gameRoom->GetRoomPwd() != "")
            gameRoom->SetPwdYn(true);

        nlohmann::json json_obj = JsonUtils::createJson(
            std::make_pair("id", gameRoom->GetRoomId()),
            std::make_pair("pwdYn",gameRoom->GetPwdYn()),
            std::make_pair("name", gameRoom->GetRoomName()),
            std::make_pair("password", gameRoom->GetRoomPwd()),
            std::make_pair("rootUser", gameRoom->GetRootUser())
        );

        std::string json_str = json_obj.dump();

        const char* query = "SET room:%d %s";
        RedisUtils::RAsyncCommand(GRedisConnection->GetContext(), query, _roomid, json_str.c_str());

        gameRoom->Init(1);
        _rooms[roomId] = gameRoom;
    }
    
    ResponseCreateRoomPacket(gameRoom,session,pkt.sessionid());

    return gameRoom;
}

bool RoomManager::Remove(int32 roomId)
{

    return _rooms.erase(roomId);

}

RoomRef RoomManager::Find(int32 roomId)
{
    RoomRef gameRoom = nullptr;
    for (auto it = _rooms.begin(); it != _rooms.end(); ++it)
    {
        if (it->first == roomId)
            gameRoom = it->second;
    }
    return gameRoom;
}


void RoomManager::RequestCreateRoomFromRedis(const ServerProtocol::C_CREATE_ROOM& pkt, UserServerSessionRef session)
{
    const char* query = "INCR room:id:counter";

    // 복사해둘 데이터 캡처
    std::string roomName = pkt.roomname();
    std::string roomPwd = pkt.roompwd();
    std::string rootUser = pkt.rootuser();
    int32 mapId = 1;

    auto onRoomIdReceived = [this, pkt,session](void* reply)
    {
        redisReply* r = static_cast<redisReply*>(reply);

        if (!r || r->type != REDIS_REPLY_INTEGER)
        {
            std::cerr << "Redis error: room id not received." << std::endl;
            return;
        }

        const int32 roomId = static_cast<int32>(r->integer);

        // 실제 방 생성
        RoomRef room = this->Add(pkt , roomId, session);
    };

    RedisUtils::RAsyncCommand(
        GRedisConnection->GetContext(),
        onRoomIdReceived,
        query
    );
}

void RoomManager::ResponseCreateRoomPacket(RoomRef room, UserServerSessionRef session, int32 clientSessionId)
{
    ServerProtocol::S_CREATE_ROOM pkt;
    room == nullptr ? pkt.set_result(false) : pkt.set_result(true);
    pkt.set_roomid(room->GetRoomId());
    pkt.set_roomname(room->GetRoomName());
    pkt.set_roompwd(room->GetRoomPwd());
    pkt.set_pwdyn(room->GetPwdYn());
    pkt.set_rootuser(room->GetRootUser());
    pkt.set_sessionid(clientSessionId);
    
    SendBufferRef Packet = RoomPacketHandler::MakeSendBuffer(pkt);
    session->Send(Packet);
}

void RoomManager::DoRoomUpdate()
{
    if (_rooms.empty())
        return;
}

