#include "pch.h"
#include "Room.h"
#include "RoomManager.h"
#include "ServerProtocol.pb.h"



RoomRef RoomManager::Add(int32 mapId, string name, string pwd, string rootUser)
{
    RoomRef gameRoom = Make_Shared<Room>();

    WRITE_LOCK
    {
   
    }
    return gameRoom;
}

//이미 만들어저있는 방을 생성하는 add 오버로딩
void RoomManager::Add(int32 mapId, string name, string pwd, int32 Roomid, string rootUser)
{
 
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
}

