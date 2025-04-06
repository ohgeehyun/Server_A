#pragma once

class UserServerSession;

class SessionManager
{
public:
    void Add(UserServerSessionRef session);
    void Remove(UserServerSessionRef session);
    void Broadcast(SendBufferRef sendBuffer);

private:
    USE_LOCK;
    Set<UserServerSessionRef> _sessions;
};

extern SessionManager* UserServerSessionManager;