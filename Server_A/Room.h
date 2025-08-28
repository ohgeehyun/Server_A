#pragma once
#include "Protocol.pb.h"
#include "MapManager.h"
#include "JobQueue.h"
#include "Protocol.pb.h"
#include "ServerProtocol.pb.h"

enum : int32 {
    MAX_USER_COUNT = 4,
};

class Room : public JobQueue 
{
public:
    Room();
    ~Room();

    int32    GetRoomId() { return _roomId; }
    void     SetRoomId(int32 roomId) { _roomId = roomId; }
    string   GetRoomName() { return _roomName; };
    void     SetRoomName(string name) { _roomName = name; }
    string   GetRoomPwd() { return _roompwd; }
    void     SetRoomPwd(string pwd) { _roompwd = pwd; }
    string   GetRootUser() { return _rootUser; }
    void     SetRootUser(string rootUser) { _rootUser = rootUser; }
    int32    GetPlayerCount() { return (int32)_players.size(); }

    void     SetPwdYn(bool pwdYn) { _pwdYn = pwdYn; }
    bool     GetPwdYn() { return _pwdYn; }

    //외부에서 해당 유저의 존재를 알고싶을때
    PlayerRef GetPlayer(string userid);
    PlayerRef GetPlayer(int32 objectid);

    void     EnterRoom_Player(int32 objectid,PlayerRef player);

    void     Broadcast(SendBufferRef buffer);
    void     BroadcastExcept(SendBufferRef buffer, int32 objectid);
    void     Send(const string& userid, SendBufferRef buffer);
    void     HandleMoveEvent(int32 objectid,SendBufferRef buffer);

    void     LeaveGame(ServerProtocol::S_LEAVE_GAME& pkt);
    void     LeaveGame_Send(PlayerRef player,int32 objectid);
    void     ExitGameEventSend(PlayerRef player);
    void     DeSawnPacketSend(ServerProtocol::S_DESPAWN& pkt);

    void     RoomBreak(PlayerRef player);

    RoomRef  GetRoomSharedPtr() { return static_pointer_cast<Room>(shared_from_this()); };

public:
    void     EnterGame(int32 roomId,GameObjectRef object,GameSessionRef ClientSession);

  
private:
    int32  _roomId;
    string _roomName;
    string _roompwd;
    string _rootUser;
    bool   _pwdYn = false;

    HashMap<int32, PlayerRef>      _players;
    HashMap<string, PlayerRef>     _userIdToPlayers;
};