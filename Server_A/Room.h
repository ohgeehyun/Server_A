#pragma once
#include "Protocol.pb.h"
#include "MapManager.h"

class Room : public enable_shared_from_this<Room>
{
public:
    Room();
    ~Room();

    void EnterGame(GameObjectRef gameObject);
    void LeaveGame(int PlayerId);
    void Broadcast(SendBufferRef sendBuffer);
    void HandleMove(PlayerRef& player, Protocol::C_MOVE& pkt);
    void HandleSkill(PlayerRef& player, Protocol::C_SKILL& pkt);
    void Init(int32 mapId);

    MapManager GetMap() { return _map; }

    int32 GetRoomId() { return _roomId; }
    void SetRoomId(int32 RoomId) { _roomId = RoomId; }
    void TileUpdate();

private:
    int32 _roomId;
    USE_LOCK;
    HashMap<int32,PlayerRef> _players;
    HashMap<int32,MonsterRef> _monsters;
    HashMap<int32,ProjectTileRef> _projectTiles;
    MapManager _map;
};