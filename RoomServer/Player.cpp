#include "pch.h"
#include "Player.h"
#include "RedisConnection.h"
#include "UserServerSession.h"
#include "ObjectManager.h"
Player::Player()
{
    SetGameObjectType(ServerProtocol::PLAYER);
}

Player::~Player()
{
}

void Player::OnDameged(GameObjectRef attacker, int32 damege) 
{
  

    std::cout << "Playerdamege :" << damege << endl;
}

void Player::OnDead(GameObjectRef attacker)
{

}


