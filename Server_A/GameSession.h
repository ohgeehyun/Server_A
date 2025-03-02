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

    string GetUserId() { return _userid; }
    string GetNickName() { return _nickname; }
private:
    PlayerRef _myplayer;
    string _userid;
    string _nickname;
};