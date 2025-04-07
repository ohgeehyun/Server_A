#include "pch.h"
#include "ServerSessionManager.h"
#include "ServerSession.h"

void ServerSessionManager::Add(ServerSessionRef session)
{
    WRITE_LOCK;
    _sessions.insert(session);
}

void ServerSessionManager::Remove(ServerSessionRef session)
{
    WRITE_LOCK;
    _sessions.erase(session);
}

void ServerSessionManager::Broadcast(SendBufferRef sendBuffer)
{
    WRITE_LOCK;
    for (ServerSessionRef session : _sessions)
    {
        session->Send(sendBuffer);
    }
}
