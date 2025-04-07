#pragma once

class ServerSession;

class ServerSessionManager
{
public:
    void Add(ServerSessionRef session);
    void Remove(ServerSessionRef session);
    void Broadcast(SendBufferRef sendBuffer);
private:
    USE_LOCK;
    Set<ServerSessionRef> _sessions;
};

extern ServerSessionManager* GServerSessionManager;

