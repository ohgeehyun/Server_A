#include "pch.h"
#include "Player.h"


Player::Player()
{
    SetGameObjectType(Protocol::PLAYER);
}

Player::~Player()
{
}

void Player::OnDameged(GameObjectRef attacker, int damege) 
{
    GameObject::OnDameged(attacker, damege);
    
    cout << "Playerdamege :" << damege << endl;
}

void Player::OnDead(GameObjectRef attacker)
{
    GameObject::OnDead(attacker);
    cout << "Object Dead" << endl;
}


