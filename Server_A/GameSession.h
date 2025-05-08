#pragma once


class GameSession : public PacketSession
{
public:
    PlayerRef GetPlayer() { return _myplayer; }
    void SetPlayer(PlayerRef player) { _myplayer = player; }
    void InitPlayer();
    void RemovePlayer() { _myplayer = nullptr; };

public:
    virtual void OnConnected() override;
    virtual void OnDisConnected() override;
    virtual void OnRecvPacket(BYTE* buffer, int32 len);

    void SetSessionId(int32 id) { _sessionId = id; }
    int32 GetSessionId() { return _sessionId; }

    void SetUserId(const string userid) { _userid = userid; }
    void SetNickName(const string nickname) { _nickname = nickname; }
    
    void SetJwtToken(const string token) { _jwtToken = token; }
    string GetJwtToken() { return _jwtToken; }

    void SetIsJwtVerify(bool verify) { _isJwtVerify = verify; }
    bool GetIsJwtVerify() { return _isJwtVerify; }


    string& GetUserId() {  return _userid; }
    string& GetNickName() { return _nickname; }

    GameSessionRef GetGameSessionRef() {return static_pointer_cast<GameSession>(shared_from_this());}

private:
    PlayerRef _myplayer;
    string    _userid;
    string    _nickname;
    string    _jwtToken;
    bool      _isJwtVerify = false;

    int32     _sessionId;
};