#pragma once
#include "Room.h"
#include "Protocol.pb.h"
#include "GameObject.h"

class Player : public GameObject
{
public:
    Player();
    ~Player();

public:
    GameSessionRef& GetSession() { return _session; }
    void SetSession(GameSessionRef session) { _session = session; }
    void OnDameged(GameObjectRef attacker, int32 damege) override;
    void OnDead(GameObjectRef attacker) override;

public:

private:
    GameSessionRef _session;

};

