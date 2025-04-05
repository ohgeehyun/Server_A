#include "pch.h"
#include "Player.h"
#include "RedisConnection.h"
#include "GameSession.h"
#include "ObjectManager.h"
Player::Player()
{
    SetGameObjectType(Protocol::PLAYER);
}

Player::~Player()
{
}

void Player::OnDameged(GameObjectRef attacker, int32 damege) 
{
    PlayerRef player = dynamic_pointer_cast<Player>(attacker);
    RoomRef room = GetRoom();

    GameObject::OnDameged(attacker, damege);

    if (attacker->GetGameObjectType() == Protocol::PLAYER && attacker->GetObjectId()!= GetObjectId())
    {
        const char* query = "HINCRBY room_score:%d:%s TotalDamege %d";
        RedisUtils::RAsyncCommand(GRedisConnection->GetContext(), query, room->GetRoomId(), player->GetSession()->GetUserId().c_str(), damege);
    }

    std::cout << "Playerdamege :" << damege << endl;
}

void Player::OnDead(GameObjectRef attacker)
{
    PlayerRef player = static_pointer_cast<Player>(attacker);
    RoomRef room = GetRoom();
    Protocol::GameObjectType type = player->GetGameObjectType();

    GameObject::OnDead(attacker);

    const char* query = "HINCRBY room_score:%d:%s death 1";
    RedisUtils::RAsyncCommand(GRedisConnection->GetContext(), query, room->GetRoomId(), GetSession()->GetUserId().c_str());

    if (type == Protocol::PLAYER)
    {
        if (attacker->GetGameObjectType() == Protocol::PLAYER && attacker->GetObjectId() != GetObjectId())
        {
            const char* query = "HINCRBY room_score:%d:%s kill 1";
            RedisUtils::RAsyncCommand(GRedisConnection->GetContext(), query, room->GetRoomId(), player->GetSession()->GetUserId().c_str());
        }
    }

}


