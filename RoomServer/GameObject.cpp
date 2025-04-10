#include "pch.h"
#include "GameObject.h"
#include "Room.h"
#include "RoomPacketHandler.h"
#include "RoomManager.h"
#include "RedisConnection.h"


GameObject::GameObject()
{
}

Vector2Int GameObject::GetFrontCellPos(ServerProtocol::MoveDir dir)
{
    Vector2Int cellPos = GetCellPos();

    switch (dir)
    {
    case ServerProtocol::MoveDir::UP:
        cellPos += Vector2Int::up();
        break;
    case ServerProtocol::MoveDir::DOWN:
        cellPos += Vector2Int::down();
        break;
    case ServerProtocol::MoveDir::LEFT:
        cellPos += Vector2Int::left();
        break;
    case ServerProtocol::MoveDir::RIGHT:
        cellPos += Vector2Int::right();
        break;
    }

    return cellPos;
}

Vector2Int GameObject::GetFrontCellPos()
{
    return GetFrontCellPos(GetMoveDir());
}

ServerProtocol::MoveDir GameObject::GetDirFromVec(Vector2Int dir)
{
    if (dir.posx > 0)
        return ServerProtocol::MoveDir::RIGHT;
    else if (dir.posx < 0)
        return ServerProtocol::MoveDir::LEFT;
    else if (dir.posy > 0)
        return ServerProtocol::MoveDir::UP;
    else
        return ServerProtocol::MoveDir::DOWN;
}

void GameObject::OnDameged(GameObjectRef attacker, int32 damege)
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

    ServerProtocol::S_CHANGEHP changehpPacket;
    changehpPacket.set_objectid(GetObjectId());
    changehpPacket.set_hp(GetHp());
    auto changehpPacketBuffer = RoomPacketHandler::MakeSendBuffer(changehpPacket);

    GetRoom()->DoAsync(&Room::Broadcast,changehpPacketBuffer);
}

void GameObject::OnDead(GameObjectRef attacker)
{
    if (GetRoom() == nullptr)
        return;
    //TODO : 죽을 때 위치를 초기화해주어야함 특히 몬스터부분 확인 해볼 것

    ServerProtocol::S_DIE diePacket;
    diePacket.set_objectid(GetObjectId());
    diePacket.set_attackerid(attacker->GetObjectId());
    auto diePacketBuffer = RoomPacketHandler::MakeSendBuffer(diePacket);
    GetRoom()->DoAsync(&Room::Broadcast,diePacketBuffer);

    RoomRef room = GetRoom();

    GetRoom()->DoAsync(&Room::LeaveGame,GetObjectId());

    GetRoom()->GetMap().ApplyLeave(shared_from_this());

    SetHp(GetObjectStat().maxhp());
    SetState(ServerProtocol::CreatureState::IDLE);
    SetMoveDir(ServerProtocol::MoveDir::DOWN);
    SetPosx(0);
    SetPosy(0);
    room->DoAsync(&Room::EnterGame,shared_from_this());
}

void GameObject::Update()
{
}
