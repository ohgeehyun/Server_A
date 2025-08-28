#include "pch.h"
#include "Monster.h"
#include "Protocol.pb.h"
#include "GameObject.h"
#include "Player.h"
#include "Room.h"
#include "ClientPacketHandler.h"
#include "DataContent.h"
#include "DataManager.h"
#include "RedisConnection.h"
#include "GameSession.h"


Monster::Monster()
{
    SetGameObjectType(Common::MONSTER);

    SetLevel(1);
    SetHp(100);
    SetMaxHp(100);
    SetSpeed(5.0f);
    SetState(Common::CreatureState::IDLE);
}

Monster::~Monster()
{
}

void Monster::Update()
{
    switch (GetState())
    {
    case Common::CreatureState::IDLE:
        UpdateIdle();
        break;
    case Common::CreatureState::MOVING:
        UpdateMoving();
        break;
    case Common::CreatureState::SKILL:
        UpdateSkill();
        break;
    case Common::CreatureState::DEAD:
        UpdateDead();
        break;
    }
}


void Monster::BroadCastMove()
{
  
}

void Monster::OnDead(GameObjectRef attacker)
{
    //캐릭터가 죽고 방에서 leave enter를 비동기적으로 처리 방식이라 객체와 room이 끊길수있어서 미리 변수로만들어서 사용
    RoomRef room = GetRoom();
    PlayerRef player = static_pointer_cast<Player>(attacker);

    _target = nullptr;

    GameObject::OnDead(attacker);

    const char* query = "HINCRBY room_score:%d:%s kill 1";
    //RedisUtils::RAsyncCommand(GRedisConnection->GetContext(), query, room->GetRoomId(), player->GetSession()->GetUserId().c_str());
}

void Monster::OnDameged(GameObjectRef attacker, int32 damege)
{
    PlayerRef player = static_pointer_cast<Player>(attacker);
    RoomRef room = GetRoom();

    GameObject::OnDameged(attacker, damege);

    if (attacker->GetGameObjectType() == Common::PLAYER && attacker->GetObjectId() != GetObjectId())
    {
        const char* query = "HINCRBY room_score:%d:%s TotalDamege %d";
        //RedisUtils::RAsyncCommand(GRedisConnection->GetContext(), query, room->GetRoomId(), player->GetSession()->GetUserId().c_str(), damege);
    }
}

void Monster::UpdateIdle()
{
   
}

void Monster::UpdateMoving()
{
  
}


void Monster::UpdateSkill()
{

}

void Monster::UpdateDead()
{
}
