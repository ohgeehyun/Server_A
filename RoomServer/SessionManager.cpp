#include "pch.h"
#include "SessionManager.h"
#include "UserServerSession.h"

void SessionManager::Add(UserServerSessionRef session)
{
    WRITE_LOCK;
    _sessions.insert(session);
}

void SessionManager::Remove(UserServerSessionRef session)
{
    WRITE_LOCK;
    _sessions.erase(session);
}

void SessionManager::Broadcast(SendBufferRef sendBuffer)
{
    WRITE_LOCK;
    for (UserServerSessionRef session : _sessions)
    {
        session->Send(sendBuffer);
    }
}
