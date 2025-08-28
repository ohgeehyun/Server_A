#include "pch.h"
#include "GameSessionManager.h"
#include "Player.h"
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

void GameSessionManager::Add_UserIdToSession(GameSessionRef session, string userid)
{
    WRITE_LOCK;
    {
        _userIdToSessions[userid] = session;
    }
}

void GameSessionManager::Add_SessionIdToSession(GameSessionRef session, int32 Sessionid)
{
    WRITE_LOCK;
    {
        _sessions[Sessionid] = session;
    }
}

GameSessionRef GameSessionManager::Find_UserIdToSession(string userid)
{
    auto it = _userIdToSessions.find(userid);
    if (it != _userIdToSessions.end())
        return it->second;

    return nullptr;
}

void GameSessionManager::Remove(int32 targetId)
{
    WRITE_LOCK;
    _sessions.erase(targetId);
}

void GameSessionManager::Remove(string userId)
{
    WRITE_LOCK;
    _userIdToSessions.erase(userId);
}

void GameSessionManager::Broadcast(SendBufferRef sendBuffer)
{
    WRITE_LOCK; // TODO : ReadLock으로 할 수 있는지 체크 해볼 것
    for (const std::pair<const int32, GameSessionRef>& pair : _sessions)
    {
        pair.second->Send(sendBuffer);
    }
}

GameSessionRef GameSessionManager::Find(int32 SessionId)
{
    WRITE_LOCK;
    auto it = _sessions.find(SessionId);
    if (it != _sessions.end())
        return it->second;

    return nullptr;
}


bool GameSessionManager::FindWaitSession(string userId)
{
    WRITE_LOCK;
    auto it = _closeWaitSession.find(userId);

    if (it == _closeWaitSession.end())
        return false;

    return true;
}

void GameSessionManager::SessionClose(int32 sessionId, string userId)
{
    WRITE_LOCK;
    {
        auto it = _userIdToSessions.find(userId);

        if (it == _userIdToSessions.end())
            return;

        _closeWaitSession[it->first] = it->second; //대기에 넣어줄 것

        Remove(sessionId);
        Remove(userId);
    }
}

void GameSessionManager::SessionDelete(string userId)
{
    WRITE_LOCK;
    {
        auto it = _closeWaitSession.find(userId);
        if (it == _closeWaitSession.end())
            return;

        GameSessionRef session = it->second;

        //세션이 물고잇는 shared_ptr 풀어 줄 것 
        session->GetPlayer()->RemoveSession();
        session->RemovePlayer();
        
        _closeWaitSession.erase(userId);
    }
}

void GameSessionManager::TransferSessionData(string userid,GameSessionRef& session,string jwt)
{
    WRITE_LOCK;
    {
        auto it = _closeWaitSession.find(userid);

        if (it != _closeWaitSession.end())
            return;

        //소켓 생성시 자동으로 _sessions에 등록된 것 삭제
        Remove(session->GetSessionId());

        GameSessionRef prevSession = it->second;
        session->SetUserId(prevSession->GetUserId());
        session->SetNickName(prevSession->GetNickName());
        session->SetJwtToken(jwt);
        session->SetIsJwtVerify(true);
        session->SetPlayer(prevSession->GetPlayer());
        session->SetSessionId(prevSession->GetSessionId());
        Add_SessionIdToSession(session, session->GetSessionId());
        Add_UserIdToSession(session, prevSession->GetUserId());
    }
}

