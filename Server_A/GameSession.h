#pragma once


class GameSession : public PacketSession
{
public:
    PlayerRef& GetPlayer() { return _myplayer; }
    void SetPlayer(PlayerRef player) { _myplayer = player; }

public:
    virtual void OnConnected() override;
    virtual void OnDisConnected() override;
    virtual void OnRecvPacket(BYTE* buffer, int32 len);

public:
    PlayerRef _myplayer;

};