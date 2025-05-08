#pragma once

class RoomSession;

class RoomSessionManager
{
public:
    void Add(RoomSessionRef session);
    void Remove(RoomSessionRef session);
    void Broadcast(SendBufferRef sendBuffer);

    RoomSessionRef Pop_Session();

private:
    USE_LOCK;
    Set<RoomSessionRef> _sessions;
};

extern RoomSessionManager* GRoomSessionManager;

