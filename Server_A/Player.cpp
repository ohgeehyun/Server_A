#include "pch.h"
#include "Player.h"


Player::Player()
{
    SetGameObjectType(Protocol::PLAYER);
    SetSpeed(20.0f);
}

Player::~Player()
{
}

void Player::OnDamaged(GameObjectRef attacker, int damege)
{
    cout <<"TODO : damege :"<< damege << endl;
}


