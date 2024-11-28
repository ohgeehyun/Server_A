#include "pch.h"
#include "GameObject.h"



GameObject::GameObject()
{
    SetObjectId(0); // 기본값 설정 (필요 시 나중에 변경)
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

void GameObject::OnDamaged(GameObjectRef attacker,int damege)
{

}
