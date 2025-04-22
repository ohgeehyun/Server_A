#pragma once


class GameSession : public PacketSession
{
public:
    PlayerRef& GetPlayer() { return _myplayer; }
    void InitPlayer();

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

    //c_str()호출시string에 size_max string이 redis비동기호출시 할당되는경우가있어(확실하지 않음 size_max 가 되는건 확실) 추적하기위해 잠시 출력문 삽입
    string& GetUserId() { cout << "userid size : " << _userid.size() << endl; return _userid; }
    string& GetNickName() { return _nickname; }

private:
    PlayerRef _myplayer;
    string    _userid;
    string    _nickname;
    string    _jwtToken;
    bool      _isJwtVerify = false;

    int32     _sessionId;
};