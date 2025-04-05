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

    void SetUserId(const string userid) { _userid = userid; }
    void SetNickName(const string nickname) { _nickname = nickname; }
    
    void SetJwtToken(const string token) { _jwtToken = token; }
    string GetJwtToken() { return _jwtToken; }

    void SetIsJwtVerify(bool verify) { _isJwtVerify = verify; }
    bool GetIsJwtVerify() { return _isJwtVerify; }

    string& GetUserId() { cout << "userid size : " << _userid.size() << endl; return _userid; }
    string& GetNickName() { return _nickname; }
private:
    PlayerRef _myplayer;
    string _userid;
    string _nickname;
    string _jwtToken;
    bool _isJwtVerify = false;
};