#pragma once

class RoomManager
{
public:
    static RoomManager& GetInstance() {
        static RoomManager instance;
        return instance;
    }

    RoomRef Add(const ServerProtocol::C_CREATE_ROOM& pkt,int32 roomId, UserServerSessionRef session);
    bool    Remove(int32 roomId);
    const RoomRef& Find(int32 roomId) const;

    //redis에서 방 번호를 받아온 뒤 방 생성
    void RequestCreateRoomFromRedis(const ServerProtocol::C_CREATE_ROOM& pkt, UserServerSessionRef session);
    //방 생성 완료 후 방 생성 요청을한 UserServer에게 response
    void ResponseCreateRoomPacket(RoomRef room, UserServerSessionRef session,int32 clientSessionId);

    void DoRoomUpdate();

private:
    RoomManager() = default;
    RoomManager(const RoomManager&) = delete;
    RoomManager& operator=(const RoomManager&) = delete;

    USE_LOCK;
    HashMap<int32, RoomRef> _rooms;
    int32 _roomid = 1;
};