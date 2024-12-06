#pragma once
#include "Protocol.pb.h"
#include "MapManager.h"
#include "JobSerializer.h"

class Room : public JobSerializer ,public enable_shared_from_this<Room>
{
public:
    Room();
    ~Room();

    void EnterGame(GameObjectRef gameObject);
    void LeaveGame(int32 PlayerId);
    void Broadcast(SendBufferRef sendBuffer);
    void HandleMove(PlayerRef& player, Protocol::C_MOVE& pkt);
    void HandleSkill(PlayerRef& player, Protocol::C_SKILL& pkt);
    void Init(int32 mapId);

    MapManager& GetMap() { return _map; }

    int32 GetRoomId() { return _roomId; }
    void SetRoomId(int32 RoomId) { _roomId = RoomId; }
    void Update();

    //FindPlayer는 Room이아닌 외부에서 FindPlayer를 사용하면 멀티스레드환경에서 안전하지않음.
    PlayerRef FindPlayer(std::function<bool(const GameObjectRef&)>condition);
  

private:
    int32 _roomId;
    MapManager _map;
   
    HashMap<int32,PlayerRef> _players;
    HashMap<int32,MonsterRef> _monsters;
    HashMap<int32,ProjectTileRef> _projectTiles;
    
};