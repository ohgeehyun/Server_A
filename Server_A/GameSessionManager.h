#pragma once

class GameSession;


class GameSessionManager
{
public:
    void Add(GameSessionRef session);
    void Remove(int32 targetId);
    void Broadcast(SendBufferRef sendBuffer);
    GameSessionRef Find(int32 targetId);

private:
    USE_LOCK;
    HashMap<int32,GameSessionRef> _sessions;
    Atomic<int32> _sessionId = 1;
};
extern GameSessionManager* GGameSessionManager;