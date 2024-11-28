#include "pch.h"
#include "Arrow.h"
#include "Room.h"
#include "ClientPacketHandler.h"

Arrow::Arrow()
{

}

Arrow::~Arrow()
{
}

void Arrow::Update() 
{
    if (GetSkillData().id == 0 || _owner == nullptr || GetRoom() == nullptr)
        return;

    if (_nextMoveTick >= GetTickCount64())
        return;
    long tick = (long)(1000 / GetSkillData().projectile.speed);
    _nextMoveTick = GetTickCount64() + tick;

    Vector2Int destPos = GetFrontCellPos();
    if (GetRoom()->GetMap().CanGo(destPos))
    {
        SetCellPos(destPos.posx, destPos.posy);

        Protocol::S_MOVE movePacket;
        movePacket.set_objectid(GetObjectId());
        movePacket.mutable_posinfo()->set_movedir(GetMoveDir());
        movePacket.mutable_posinfo()->set_posx(GetPosx());
        movePacket.mutable_posinfo()->set_posy(GetPosy());
        movePacket.mutable_posinfo()->set_state(GetState());
        
        auto movePacketBuffer = ClientPacketHandler::MakeSendBuffer(movePacket);
        GetRoom()->Broadcast(movePacketBuffer);

        cout<<"Move Arrow"<< endl;
    }
    else
    {
        GameObjectRef target = GetRoom()->GetMap().Find(destPos);
        if (target != nullptr)
        {
            // TODO :  피격 판정
            target->OnDamaged(shared_from_this(), GetSkillData().damege);
        }

        //소멸
        GetRoom()->LeaveGame(GetObjectId());
    }
}
