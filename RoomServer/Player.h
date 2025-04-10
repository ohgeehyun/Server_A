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
    UserServerSessionRef& GetSession() { return _session; }
    void SetSession(UserServerSessionRef session) { _session = session; }
    void OnDameged(GameObjectRef attacker, int32 damege) override;
    void OnDead(GameObjectRef attacker) override;

public:

private:
    UserServerSessionRef _session;

};

