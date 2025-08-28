#include "pch.h"
#include "GameObject.h"
#include "Room.h"
#include "RoomPacketHandler.h"
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
    changehpPacket.set_roomid(GetRoom()->GetRoomId());
    auto changehpPacketBuffer = RoomPacketHandler::MakeSendBuffer(changehpPacket);

    GetRoom()->DoAsync(&Room::Broadcast,std::move(changehpPacketBuffer));
}

void GameObject::OnDead(GameObjectRef attacker)
{
    if (GetRoom() == nullptr)
        return;
    // 죽을 때 위치를 초기화해주어야함 특히 몬스터부분 확인 해볼 것

    ServerProtocol::S_DIE diePacket;
    diePacket.set_objectid(GetObjectId());
    diePacket.set_attackerid(attacker->GetObjectId());
    diePacket.set_roomid(GetRoom()->GetRoomId());
    auto diePacketBuffer = RoomPacketHandler::MakeSendBuffer(diePacket);
    GetRoom()->DoAsync(&Room::Broadcast,std::move(diePacketBuffer));

    RoomRef room = GetRoom();

    GetRoom()->DoAsync(&Room::LeaveGame,GetObjectId());

    GetRoom()->GetMap().ApplyLeave(shared_from_this());

    SetHp(GetObjectStat().maxhp());
    SetState(Common::CreatureState::IDLE);
    SetMoveDir(Common::MoveDir::DOWN);
    SetPosx(0);
    SetPosy(0);
    room->DoAsync(&Room::EnterGame,shared_from_this());
}

void GameObject::Update()
{
}
