#pragma once

class GameSession;


class GameSessionManager
{
public:
    void Add(GameSessionRef session);
    void Add_UserIdToSession(GameSessionRef session,string userid);
    void Add_SessionIdToSession(GameSessionRef session, int32 Sessionid);

    GameSessionRef Find(int32 SessionId);
    GameSessionRef Find_UserIdToSession(string userid);
  
    void Remove(int32 targetId);
    void Remove(string userId);
    void Broadcast(SendBufferRef sendBuffer);


    //_closeWaitSession관련
    bool FindWaitSession(string userId);
    void SessionClose(int32 sessionId,string userId);
    void SessionDelete(string userId);

    void TransferSessionData(string userid,GameSessionRef& session,string jwt);
private:
    USE_LOCK;
    HashMap<int32,GameSessionRef> _sessions;
    HashMap<string, GameSessionRef> _userIdToSessions;
    Atomic<int32> _sessionId = 1;

    //종료시 세션을 바로 삭제하는 것이 아닌 WaitSession에 보관 shared_ptr의 생명주기 최후의 보루 장소
    HashMap<string, GameSessionRef> _closeWaitSession;
};
extern GameSessionManager* GGameSessionManager;