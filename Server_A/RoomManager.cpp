#include "pch.h"
#include "Room.h"
#include "RoomManager.h"



RoomRef RoomManager::Add(int32 mapId)
{
    RoomRef gameRoom = Make_Shared<Room>();
   
    std::function<void(int32)> job = [gameRoom](int32 mapId) {
        gameRoom->Init(mapId);
    };
    gameRoom->Push(job,mapId);
   
    WRITE_LOCK
    {
       gameRoom->SetRoomId(_roomid);
       _rooms[_roomid] = gameRoom;
       _roomid++;
    }

    return gameRoom;
}

bool RoomManager::Remove(int32 roomId)
{
    WRITE_LOCK
    {
         return _rooms.erase(roomId);
    }
}

RoomRef RoomManager::Find(int32 roomId)
{
    WRITE_LOCK
    {
        RoomRef gameRoom = nullptr;
        for (auto it = _rooms.begin(); it != _rooms.end(); ++it) 
        {
            if (it->first == roomId)
                gameRoom = it->second;
        }
        return gameRoom;
    }
}
