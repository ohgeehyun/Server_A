#include "pch.h"
#include "GameObject.h"
#include "Room.h"
#include "ClientPacketHandler.h"
#include "RoomManager.h"
#include "RedisConnection.h"


GameObject::GameObject()
{
}

Vector2Int GameObject::GetFrontCellPos(Protocol::MoveDir dir)
{
    Vector2Int cellPos = GetCellPos();

    switch (dir)
    {
    case Protocol::MoveDir::UP:
        cellPos += Vector2Int::up();
        break;
    case Protocol::MoveDir::DOWN:
        cellPos += Vector2Int::down();
        break;
    case Protocol::MoveDir::LEFT:
        cellPos += Vector2Int::left();
        break;
    case Protocol::MoveDir::RIGHT:
        cellPos += Vector2Int::right();
        break;
    }

    return cellPos;
}

Vector2Int GameObject::GetFrontCellPos()
{
    return GetFrontCellPos(GetMoveDir());
}

Protocol::MoveDir GameObject::GetDirFromVec(Vector2Int dir)
{
    if (dir.posx > 0)
        return Protocol::MoveDir::RIGHT;
    else if (dir.posx < 0)
        return Protocol::MoveDir::LEFT;
    else if (dir.posy > 0)
        return Protocol::MoveDir::UP;
    else
        return Protocol::MoveDir::DOWN;
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
