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

void GameObject::OnDameged(GameObjectRef attacker,int damege)
{
    if (GetRoom() == nullptr)
        return;

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
    if (GetRoom() == nullptr)
        return;

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

    room->EnterGame(shared_from_this());   
}  

void GameObject::Update()
{
}


