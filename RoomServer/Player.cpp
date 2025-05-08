#include "pch.h"
#include "Player.h"
#include "RedisManager.h"
#include "UserServerSession.h"
#include "ObjectManager.h"
Player::Player()
{
    SetGameObjectType(Common::PLAYER);
}

Player::~Player()
{
    cout << "마법 오브젝트 소멸자 호출 완료" << endl;
}

void Player::OnDameged(GameObjectRef attacker, int32 damege) 
{
    PlayerRef player = dynamic_pointer_cast<Player>(attacker);
    RoomRef room = GetRoom();

    GameObject::OnDameged(attacker, damege);

    if (attacker->GetGameObjectType() == Common::PLAYER && attacker->GetObjectId() != GetObjectId())
    {
        const char* query = "HINCRBY room_score:%d:%s TotalDamege %d";
        RedisManager::GetInstance().RAsyncCommand(query, room->GetRoomId(), player->GetUserId().c_str(), damege);
    }
    std::cout << "Playerdamege :" << damege << endl;
}

void Player::OnDead(GameObjectRef attacker)
{
    PlayerRef player = static_pointer_cast<Player>(attacker); 
    RoomRef room = GetRoom();
    Common::GameObjectType type = player->GetGameObjectType();

    GameObject::OnDead(attacker);

    const char* query = "HINCRBY room_score:%d:%s death 1";
    RedisManager::GetInstance().RAsyncCommand(query, room->GetRoomId(),GetUserId().c_str());

    if (type == Common::PLAYER)
    {
        if (attacker->GetGameObjectType() == Common::PLAYER && attacker->GetObjectId() != GetObjectId())
        {
            const char* query = "HINCRBY room_score:%d:%s kill 1";
            RedisManager::GetInstance().RAsyncCommand(query, room->GetRoomId(), player->GetUserId().c_str());
        }
    }
}

void Player::initPlayer(const string& userid)
{
   GetObjectInfo().set_name("Player_" + to_string(GetObjectId()));
   SetState(Common::CreatureState::IDLE);
   SetMoveDir(Common::MoveDir::DOWN);
   SetPosx(0);
   SetPosy(0);
   SetUserId(userid);

   auto it = std::find_if(DataManager::GetInstance().GetStatDict().begin(), DataManager::GetInstance().GetStatDict().end(),
        [](const std::pair<const int32, Common::STATINFO>& pair) {
        return pair.second.level() == 1;
   });

   if (it == DataManager::GetInstance().GetStatDict().end())
        return;

    SetObjectStat(it->second);
}


