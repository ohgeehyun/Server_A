#include "pch.h"
#include "Player.h"
#include "RedisConnection.h"
#include "GameSession.h"
#include "ObjectManager.h"
Player::Player()
{
    SetGameObjectType(Common::PLAYER);
}

Player::~Player()
{
    cout << "plyaer 소멸자 호출 완료 " << endl;
}




