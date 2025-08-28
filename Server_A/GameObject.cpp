#include "pch.h"
#include "GameObject.h"
#include "Room.h"
#include "ClientPacketHandler.h"
#include "RoomManager.h"
#include "RedisConnection.h"


GameObject::GameObject()
{
}

Vector2Int GameObject::GetFrontCellPos(Common::MoveDir dir)
{
    Vector2Int cellPos = GetCellPos();

    switch (dir)
    {
    case Common::MoveDir::UP:
        cellPos += Vector2Int::up();
        break;
    case Common::MoveDir::DOWN:
        cellPos += Vector2Int::down();
        break;
    case Common::MoveDir::LEFT:
        cellPos += Vector2Int::left();
        break;
    case Common::MoveDir::RIGHT:
        cellPos += Vector2Int::right();
        break;
    }

    return cellPos;
}

Vector2Int GameObject::GetFrontCellPos()
{
    return GetFrontCellPos(GetMoveDir());
}

Common::MoveDir GameObject::GetDirFromVec(Vector2Int dir)
{
    if (dir.posx > 0)
        return Common::MoveDir::RIGHT;
    else if (dir.posx < 0)
        return Common::MoveDir::LEFT;
    else if (dir.posy > 0)
        return Common::MoveDir::UP;
    else
        return Common::MoveDir::DOWN;
}

void GameObject::OnDameged(GameObjectRef attacker, int32 damege)
{
 
}

void GameObject::OnDead(GameObjectRef attacker)
{
  
}

void GameObject::Update()
{
}
