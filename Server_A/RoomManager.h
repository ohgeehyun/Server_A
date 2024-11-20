#pragma once
class RoomManager
{
public:
    static RoomManager& GetInstance() {
        static RoomManager instance;
        return instance;
    }

     RoomRef Add();
     bool Remove(int32 roomId);
     RoomRef Find(int32 roomId);

private:
    RoomManager() = default;
    RoomManager(const RoomManager&) = delete;
    RoomManager& operator=(const RoomManager&) = delete;

    USE_LOCK;
    HashMap<int32, RoomRef> _rooms;
    int32 _roomid = 1;
};

