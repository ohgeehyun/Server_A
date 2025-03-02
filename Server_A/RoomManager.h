#pragma once

//RoomManager에서 호출하기떄문에 lock은걸지않음
class RoomIdManager
{
public:
    RoomIdManager();
    ~RoomIdManager();

    int32 Add_Room();
    void Add_Room(int32 Roomid);

    void Delete_Room(int32 Roomid);
    int32 GetRoomId() { return *_roomids.begin(); }

private:
    Set<int32> _roomids;
    Set<int32> use_room;
};

class RoomManager
{
public:
    static RoomManager& GetInstance() {
        static RoomManager instance;
        return instance;
    }

    RoomRef Add(int32 mapId, string name, string pwd,string rootUser);
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
    RoomIdManager roomid;
};

