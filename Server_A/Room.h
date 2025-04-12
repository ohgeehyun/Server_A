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

    int32 GetRoomId() { return _roomId; }
    void SetRoomId(int32 RoomId) { _roomId = RoomId; }
    string GetRoomName() { return _roomName; };
    void SetRoomName(string name) { _roomName = name; }
    string GetRoomPwd() { return _roompwd; }
    void SetRoomPwd(string pwd) { _roompwd = pwd; }
    string GetRootUser() { return _rootUser; }
    void SetRootUser(string rootUser) { _rootUser = rootUser; }
    int32 GetPlayerCount() { return (int32)_players.size(); }

    void SetPwdYn(bool pwdYn) { _pwdYn = pwdYn; }
    bool GetPwdYn() { return _pwdYn; }
  
private:
    int32 _roomId;
    string _roomName;
    string _roompwd;
    string _rootUser;
    bool _pwdYn = false;

    HashMap<int32, PlayerRef> _players;
    HashMap<int32, MonsterRef> _monsters;
    HashMap<int32, ProjectTileRef> _projectTiles;
};