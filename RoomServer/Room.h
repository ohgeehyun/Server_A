#pragma once
#include "ServerProtocol.pb.h"
#include "MapManager.h"
#include "ObjectManager.h"

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
    void Send(PacketSessionRef session,SendBufferRef sendBuffer);

    void SpawnMonster(int32 y, int32 x);
    
    void LeaveGameEventSend_Player(PlayerRef player, int32 objectid);
    void LeaveGameEventSend_Monster(MonsterRef monster, int32 objectid);
    void LeaveGameEventSend_ProjectTile(ProjectTileRef projectTile, int32 objectid);

    void HandleMove(PlayerRef& player, ServerProtocol::C_MOVE& pkt);
    void HandleMoveEvent(PlayerRef player, ServerProtocol::C_MOVE pkt);

    void HandleSkill(PlayerRef& player, ServerProtocol::C_SKILL& pkt);
    void Init(int32 mapId);

    void RoomBreak(PlayerRef player);

    MapManager& GetMap() { return _map; }
    int32       GetRoomId() { return _roomId; }
    void        SetRoomId(int32 RoomId) { _roomId = RoomId; }
    string      GetRoomName() { return _roomName; };
    void        SetRoomName(string name) { _roomName = name; }
    string      GetRoomPwd() { return _roompwd; }
    void        SetRoomPwd(string pwd) { _roompwd = pwd; }
    string      GetRootUser() { return _rootUser; }
    void        SetRootUser(string rootUser) { _rootUser = rootUser; }
    int32       GetPlayerCount() { return (int32)_players.size(); }

    void        SetPwdYn(bool pwdYn) { _pwdYn = pwdYn; }
    bool        GetPwdYn() { return _pwdYn; }

    HashMap<int32, MonsterRef> GetMonsters() { return _monsters; }

    RoomRef GetSharedRoomPtr() { return static_pointer_cast<Room>(shared_from_this()); }

    //방 내부에서 사용하기 위한 함수 ex 방 내부의 오브젝트가 특정 오브젝트를 찾아야할때.
    PlayerRef FindPlayer(std::function<bool(const GameObjectRef&)>condition);

    //외부에서 해당 유저의 존재를 알고싶을때
    PlayerRef GetPlayer(string userid);
    PlayerRef GetPlayer(int32 objectid);

    ObjectManager& GetObjManager() { return _objmanagers; };
    
    void Update();

private:

    bool tempSpawnHandle = false;
    ObjectManager _objmanagers;

    MapManager _map;

    int32 _roomId;
    string _roomName = "";
    string _roompwd = "";
    string _rootUser = "";
    bool _pwdYn = false;

    UserServerSessionRef _session;

    HashMap<int32, PlayerRef> _players;
    HashMap<string, PlayerRef> _useridToPlayers;
    HashMap<int32, MonsterRef> _monsters;
    HashMap<int32, ProjectTileRef> _projectTiles;
};