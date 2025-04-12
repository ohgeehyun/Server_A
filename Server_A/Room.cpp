#include "pch.h"
#include "Room.h"
#include "GameObject.h"
#include "Player.h"
#include "Monster.h"
#include "Protocol.pb.h"
#include "ClientPacketHandler.h"
#include "GameSession.h"
#include "ObjectManager.h"
#include "DataManager.h"
#include "Monster.h"
#include "Arrow.h"
#include "MagicSkill.h"
#include "RoomManager.h"
#include "RedisConnection.h"
#include  "ServerSession.h"
#include <httplib/httplib.h> 

Room::Room() 
{
    
}

Room::~Room()
{

    const char* query = "DEL room:%d";
    RedisUtils::RAsyncCommand(GRedisConnection->GetContext(), query, _roomId);

    query = "DEL room_user:%d";
    RedisUtils::RAsyncCommand(GRedisConnection->GetContext(), query, _roomId);

    cout << _roomId << " 번 방 소멸자 호출 완료. Redis 삭제 호출 완료" << endl;
}


