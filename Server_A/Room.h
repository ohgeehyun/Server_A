#pragma once
#include "Protocol.pb.h"
#include "MapManager.h"
#include "JobQueue.h"

enum : int32 {
    MAX_USER_COUNT = 4,
};

class Room : public JobQueue
{
public:
    Room();
    ~Room();

    void EnterGame(GameObjectRef gameObject);
    
    void EnterGame_Player(PlayerRef player);
    void EnterGame_Monster(MonsterRef monster);
    void EnterGame_ProjectTile(ProjectTileRef projectTile);

    void EnterGameEventSend_Player(PlayerRef player);
    void EnterGameEventSend_Monster(MonsterRef monster);
    void EnterGameEventSend_ProjectTile(ProjectTileRef projectTile);

    void LeaveGame(int32 PlayerId);
    void LeaveGame_Player(int32 objectId);
    void LeaveGame_Monster(int32 objectId);
    void LeaveGame_ProjectTile(int32 objectId);

    void ExitGameEventSend(PlayerRef player);

    void Broadcast(SendBufferRef sendBuffer);
    void BroadcastExcept(SendBufferRef sendBuffer,PlayerRef player);
    void SpawnMonster(int32 y,int32 x);

    void LeaveGameEventSend_Player(PlayerRef player,int32 objectid);
    void LeaveGameEventSend_Monster(MonsterRef monster, int32 objectid);
    void LeaveGameEventSend_ProjectTile(ProjectTileRef projectTile, int32 objectid);

    void HandleMove(PlayerRef& player, Protocol::C_MOVE& pkt);
    void HandleMoveEvent(PlayerRef player, Protocol::C_MOVE pkt);

    void HandleSkill(PlayerRef& player, Protocol::C_SKILL& pkt);
    void Init(int32 mapId);

    void RoomBreak(PlayerRef player);

    MapManager& GetMap() { return _map; }

    int32 GetRoomId() { return _roomId; }
    void SetRoomId(int32 RoomId) { _roomId = RoomId; }
    string GetRoomName() { return _roomName; };
    void SetRoomName(string name) { _roomName = name; }
    string GetRoomPwd() { return _roompwd; }
    void SetRoomPwd(string pwd) { _roompwd = pwd; }
    string GetRootUser() { return _rootUser; }
    void SetRootUser(string rootUser) { _rootUser = rootUser; }
    int32 GetPlayerCount() { return (int32)_players.size(); }

    HashMap<int32, MonsterRef> GetMonsters() { return _monsters; }    

    RoomRef GetSharedRoomPtr() { return static_pointer_cast<Room>(shared_from_this()); }

    void Update();

    //FindPlayer는 Room이아닌 외부에서 사용시 lock 을 고려해서 사용
    PlayerRef FindPlayer(std::function<bool(const GameObjectRef&)>condition);
  

private:

    bool tempSpawnHandle = false;
    int32 _roomId;
  
    MapManager _map;
    string _roomName;
    string _roompwd;
    string _rootUser;

    HashMap<int32, PlayerRef> _players;
    HashMap<int32, MonsterRef> _monsters;
    HashMap<int32, ProjectTileRef> _projectTiles;
};