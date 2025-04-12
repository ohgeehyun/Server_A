#pragma once
#include "ServerProtocol.pb.h"

class RoomManager
{
public:
    static RoomManager& GetInstance() {
        static RoomManager instance;
        return instance;
    }

    RoomRef Add(ServerProtocol::S_CREATE_ROOM& pkt);
    bool Remove(int32 roomId);
    RoomRef Find(int32 roomId);
    void DoRoomUpdate();

private:
    RoomManager() = default;
    RoomManager(const RoomManager&) = delete;
    RoomManager& operator=(const RoomManager&) = delete;

    USE_LOCK;
    HashMap<int32, RoomRef> _rooms;
};

