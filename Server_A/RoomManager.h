#pragma once
#include "ServerProtocol.pb.h"

class RoomManager
{
public:
    static RoomManager& GetInstance() {
        static RoomManager instance;
        return instance;
    }

    RoomRef          Add(const ServerProtocol::S_CREATE_ROOM& pkt);
    bool             Remove(int32 roomId);
    RoomRef          Find(int32 roomId);
    ServerSessionRef FindToSession(int32 roomId);

    //Redis 에서 방 정보 검색.
    void             FindToRoomServerInfo_Connect(const int32 roomId,const int32& sessionId);
    void    Add_RoomIdToServerSession(const int32& roomId, const ServerSessionRef& session);

    void    DoRoomUpdate();

private:
    RoomManager() = default;
    RoomManager(const RoomManager&) = delete;
    RoomManager& operator=(const RoomManager&) = delete;

    USE_LOCK;
    HashMap<int32, RoomRef> _rooms;
    HashMap<int32, ServerSessionRef> _roomIdToServerSession;
};

