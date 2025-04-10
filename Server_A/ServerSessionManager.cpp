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

ServerSessionRef ServerSessionManager::Pop_Session()
{
    //TODO : 현재는 RoomServer는 하나기 때문에 begin() 사용 RoomServer도 많이 늘어난다면 적절하게 찾아서 분배해줄 알고리즘 추가
    return *_sessions.begin();
}
