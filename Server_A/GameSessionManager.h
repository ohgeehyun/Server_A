#pragma once

class GameSession;


class GameSessionManager
{
public:
    void Add(GameSessionRef session);
    void Remove(GameSessionRef session);
    void Broadcast(SendBufferRef sendBuffer);

private:
    USE_LOCK;
    Set<GameSessionRef> _sessions;
};
extern GameSessionManager* SessionManager;