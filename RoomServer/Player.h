#pragma once
#include "Room.h"
#include "ServerProtocol.pb.h"
#include "GameObject.h"

class Player : public GameObject
{
public:
    Player();
    ~Player();

public:
    PacketSessionRef GetSession() { return _session; }
    void SetSession(PacketSessionRef session) { _session = session; }

    const string& GetUserId() { return _userid; }
    void SetUserId(string userid) { _userid = userid; };

    const string& GetUserNickName() { return _nickname; }
    void SetUserNickName(string nickname) { _nickname = nickname; };
    
    void OnDameged(GameObjectRef attacker, int32 damege) override;
    void OnDead(GameObjectRef attacker) override;
    
    void initPlayer(const string& userid);
public:

private:
    PacketSessionRef _session;
    string _userid;
    string _nickname;
};

