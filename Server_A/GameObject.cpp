#include "pch.h"
#include "GameObject.h"
#include "Room.h"
#include "ClientPacketHandler.h"
#include "RoomManager.h"
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

void GameObject::OnDameged(GameObjectRef attacker,int damege)
{
    int32 objectHp = GetHp();
    objectHp -= damege;
    SetHp(objectHp);

    if (objectHp <= 0)
    {
        SetHp(0);
        OnDead(attacker);
    }
    

    Protocol::S_CHANGEHP changehpPacket;
    changehpPacket.set_objectid(GetObjectId());
    changehpPacket.set_hp(GetHp());
    auto changehpPacketBuffer = ClientPacketHandler::MakeSendBuffer(changehpPacket);

    GetRoom()->Broadcast(changehpPacketBuffer);

}

void GameObject::OnDead(GameObjectRef attacker)
{
    Protocol::S_DIE diePacket;
    diePacket.set_objectid(GetObjectId());
    diePacket.set_attackerid(attacker->GetObjectId());
    auto diePacketBuffer = ClientPacketHandler::MakeSendBuffer(diePacket);
    GetRoom()->Broadcast(diePacketBuffer);

    RoomRef room = GetRoom();
    GetRoom()->LeaveGame(GetObjectId());

    SetHp(GetObjectStat().maxhp());
    SetState(Protocol::CreatureState::IDLE);
    SetMoveDir(Protocol::MoveDir::DOWN);
    SetPosx(0);
    SetPosy(0);

   
    room->EnterGame(static_pointer_cast<GameObject>(shared_from_this()));
}


