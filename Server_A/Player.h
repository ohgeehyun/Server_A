#pragma once
#include "Room.h"
#include "Protocol.pb.h"

class Player
{
public:
    Player();
    ~Player();

public:
    RoomRef& GetRoom() { return _room; }
    void SetRoom(RoomRef room) { _room = room; }
    GameSessionRef& GetSession() { return _session; }
    void SetSession(GameSessionRef session) { _session = session; }
    Protocol::PLAYER_INFO& GetPlayerInfo() { return _playerInfo; }
    void SetPlayerInfo(Protocol::PLAYER_INFO playerinfo) { _playerInfo = playerinfo;}

private:
    RoomRef _room;
    Protocol::PLAYER_INFO _playerInfo;
    GameSessionRef _session;
};

