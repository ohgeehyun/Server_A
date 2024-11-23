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

private:
    GameSessionRef _session;

};

