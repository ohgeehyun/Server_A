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
    if (_owner == nullptr || GetRoom() == nullptr)
        return;

    if (_nextMoveTick >= GetTickCount64())
        return;

    _nextMoveTick = GetTickCount64() + 50;

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
        }

        //소멸
        GetRoom()->LeaveGame(GetObjectId());
    }
}
