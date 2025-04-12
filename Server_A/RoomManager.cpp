#include "pch.h"
#include "Room.h"
#include "RoomManager.h"
#include "RedisConnection.h"
#include "ServerProtocol.pb.h"
#include "ServerSessionManager.h"
#include "ServerPacketHandler.h"
#include "ServerSession.h"
#include "JsonUtils.h"


RoomRef RoomManager::Add(ServerProtocol::S_CREATE_ROOM& pkt)
{
    RoomRef gameRoom = Make_Shared<Room>();
    
    WRITE_LOCK
    {
        gameRoom->SetRoomId(pkt.roomid());
        gameRoom->SetRoomName(pkt.roomname());
        gameRoom->SetRootUser(pkt.rootuser());
        gameRoom->SetPwdYn(pkt.pwdyn());

        if (pkt.pwdyn())
            gameRoom->SetRoomPwd(pkt.roompwd());
        else
            gameRoom->SetRoomPwd("");

        _rooms.insert(std::pair(pkt.roomid(),gameRoom));

    }
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

void RoomManager::DoRoomUpdate()
{
    if (_rooms.empty())
        return;

    for (const auto &item : _rooms)
    {
       //item.second->DoAsync(std::bind(&Room::Update, item.second));
        item.second->Update();
    }
}

