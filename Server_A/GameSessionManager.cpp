#include "pch.h"
#include "GameSessionManager.h"
#include "GameSession.h"

void GameSessionManager::Add(GameSessionRef session)
{
    WRITE_LOCK;
    {
        session->SetSessionId(_sessionId);
        _sessions[_sessionId] = session;
        _sessionId.fetch_add(1);
    }
}

void GameSessionManager::Remove(int32 targetId)
{
    WRITE_LOCK;
    _sessions.erase(targetId);
}

void GameSessionManager::Broadcast(SendBufferRef sendBuffer)
{
    WRITE_LOCK; // TODO : ReadLock으로 할 수 있는지 체크 해볼 것
    for (const std::pair<const int32, GameSessionRef>& pair : _sessions)
    {
        pair.second->Send(sendBuffer);
    }
}

GameSessionRef GameSessionManager::Find(int32 targetId)
{
    READ_LOCK;
    auto it = _sessions.find(targetId);
    if (it != _sessions.end())
        return it->second;

    return nullptr;
}
