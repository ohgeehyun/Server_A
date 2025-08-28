#pragma once
#include "pch.h"
#include "Room.h"
#include "RoomManager.h"
#include "ServerProtocol.pb.h"
#include "RoomPacketHandler.h"
#include "UserServerSession.h"
#include "RedisConnection.h"


RoomRef RoomManager::Add(const ServerProtocol::C_CREATE_ROOM pkt, int32 roomId, PacketSessionRef& session)
{
    RoomRef gameRoom = Make_Shared<Room>();

    WRITE_LOCK
    {
        gameRoom->SetRoomId(roomId);
        gameRoom->SetRoomName(pkt.roomname());
        gameRoom->SetRoomPwd(pkt.roompwd());
        gameRoom->SetRootUser(pkt.rootuser());
        _rooms[roomId] = gameRoom;

        if (pkt.roompwd().length() > 0)
            gameRoom->SetPwdYn(true);

        gameRoom->Init(1);   

        nlohmann::json json_obj = JsonUtils::createJson(
            std::make_pair("id", gameRoom->GetRoomId()),
            std::make_pair("pwdYn", gameRoom->GetPwdYn()),
            std::make_pair("name", gameRoom->GetRoomName()),
            std::make_pair("password", gameRoom->GetRoomPwd()),
            std::make_pair("rootUser", gameRoom->GetRootUser())
        );

        std::string json_str = json_obj.dump();

        const char* query = "SET room:%d %s";
        RedisManager::GetInstance().RAsyncCommand(query, roomId, json_str.c_str());

        std::cout << "Room 번호 : " << roomId << " Room 생성 , 방 이름 : " << gameRoom->GetRoomName() << " Room pwd : " << gameRoom->GetRoomPwd() << endl;
    }
    ResponseCreateRoomPacket(gameRoom, session, pkt.sessionid());

    return gameRoom;
}

bool RoomManager::Remove(int32 roomId)
{

    return _rooms.erase(roomId);

}

const RoomRef& RoomManager::Find(int32 roomId) const
{
    RoomRef gameRoom = nullptr;
    for (auto it = _rooms.begin(); it != _rooms.end(); ++it)
    {
        if (it->first == roomId)
            gameRoom = it->second;
    }
    return gameRoom;
}


void RoomManager::RoomidToRedis_CreateRoom(const ServerProtocol::C_CREATE_ROOM& pkt, PacketSessionRef& session)
{
    const char* query = "INCR room_id_seq";

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
        DoAsync(std::bind(&RoomManager::Add, this, pkt, roomId, session));
    };

    RedisManager::GetInstance().RAsyncCommandCallback(onRoomIdReceived,query);
}

void RoomManager::ResponseCreateRoomPacket(RoomRef& room, PacketSessionRef& session, int32 clientSessionId)
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

PlayerRef RoomManager::GetUserInRoom(const int32& roomid, const string& userid)
{
    RoomRef room = Find(roomid);
    if (room == nullptr)
        return nullptr;

    return room->GetPlayer(userid);
}

void RoomManager::DoRoomUpdate()
{
    if (_rooms.empty())
        return;

    for (const auto& item : _rooms)
    {
        //item.second->DoAsync(std::bind(&Room::Update, item.second));
        item.second->Update();
    }
}

