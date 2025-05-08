#include "pch.h"
#include "RoomSessionManager.h"
#include "RoomSession.h"

void RoomSessionManager::Add(RoomSessionRef session)
{
    WRITE_LOCK;
    _sessions.insert(session);
}

void RoomSessionManager::Remove(RoomSessionRef session)
{
    WRITE_LOCK;
    _sessions.erase(session);
}

void RoomSessionManager::Broadcast(SendBufferRef sendBuffer)
{
    WRITE_LOCK;
    for (RoomSessionRef session : _sessions)
    {
        session->Send(sendBuffer);
    }
}

RoomSessionRef RoomSessionManager::Pop_Session()
{
    //TODO : 현재는 RoomServer는 하나기 때문에 begin() 사용 RoomServer도 많이 늘어난다면 적절하게 찾아서 분배해줄 알고리즘 추가
    return *_sessions.begin();
}
