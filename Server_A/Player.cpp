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
        redisAsyncCommand(GRedisConnection->GetContext(), [](redisAsyncContext* context, void* reply, void* privdata)
        {
            if (reply != nullptr)
                RedisUtils::replyResponseHandler(reply, "damege Score Add : ");

        }, nullptr, "HINCRBY room_score:%d:%s TotalDamege %d", room->GetRoomId(), player->GetSession()->GetUserId().c_str(), damege);
    }

    std::cout << "Playerdamege :" << damege << endl;
}

void Player::OnDead(GameObjectRef attacker)
{
    PlayerRef player = static_pointer_cast<Player>(attacker);
    RoomRef room = GetRoom();
    Protocol::GameObjectType type = player->GetGameObjectType();

    GameObject::OnDead(attacker);


    redisAsyncCommand(GRedisConnection->GetContext(), [](redisAsyncContext* context, void* reply, void* privdata)
    {
        if (reply != nullptr)
            RedisUtils::replyResponseHandler(reply, "death Score Add : ");

    }, nullptr, "HINCRBY room_score:%d:%s death 1", room->GetRoomId(), GetSession()->GetUserId().c_str());

    if (type == Protocol::PLAYER)
    {
        if (attacker->GetGameObjectType() == Protocol::PLAYER && attacker->GetObjectId() != GetObjectId())
        {
            redisAsyncCommand(GRedisConnection->GetContext(), [](redisAsyncContext* context, void* reply, void* privdata)
            {
                if (reply != nullptr)
                    RedisUtils::replyResponseHandler(reply, "kill Score Add : ");

            }, nullptr, "HINCRBY room_score:%d:%s kill 1", room->GetRoomId(), player->GetSession()->GetUserId().c_str());
        }
    }

    std::cout << "Object Dead" << endl;
}


