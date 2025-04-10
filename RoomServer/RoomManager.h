#pragma once

class RoomManager
{
public:
    static RoomManager& GetInstance() {
        static RoomManager instance;
        return instance;
    }

    RoomRef Add(int32 mapId, string name, string pwd, string rootUser);
    void Add(int32 mapId, string name, string pwd, int32 Roomid, string rootUser);
    bool Remove(int32 roomId);
    RoomRef Find(int32 roomId);
    void DoRoomUpdate();

private:
    RoomManager() = default;
    RoomManager(const RoomManager&) = delete;
    RoomManager& operator=(const RoomManager&) = delete;

    USE_LOCK;
    HashMap<int32, RoomRef> _rooms;
    int32 _roomid = 1;
};