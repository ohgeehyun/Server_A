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

    const PacketSessionRef& FindToSession(int32 roomId) const;

    //Redis 에서 방 정보 검색.
    void             FindToRoomServerInfo_Connect(const int32 roomId,const int32& sessionId);
    void             Add_RoomIdToServerSession(const int32& roomId, const PacketSessionRef& session);

    void             DoRoomUpdate();

private:
    RoomManager() = default;
    RoomManager(const RoomManager&) = delete;
    RoomManager& operator=(const RoomManager&) = delete;

    USE_LOCK;
    //room_id 에 따른 RoomRef 객체 매핑
    HashMap<int32, RoomRef> _rooms;
    //방번호 와 방을 소지하고있는 서버와 연결된 session 매핑
    HashMap<int32, PacketSessionRef> _roomIdToServerSession;
};

