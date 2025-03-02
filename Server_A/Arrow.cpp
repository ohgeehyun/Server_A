#include "pch.h"
#include "Arrow.h"
#include "Room.h"
#include "GameObject.h"
#include "ClientPacketHandler.h"

Arrow::Arrow()
{
    SetGameObjectType(Protocol::PROJECTTILE);
    //_skillData.id = 0;
}

Arrow::~Arrow()
{
}

void Arrow::Update()
{
    if (GetSkillData().id == 0 || _owner == nullptr || GetRoom() == nullptr)
        return;


    GameObjectRef object = GetRoom()->GetMap().Find(Vector2Int(GetPosx(), GetPosy()));

    //화살의 현재위치에 오브젝트가있을경우
    if (object != nullptr)
    {
        GetRoom()->DoAsync([this, object]() {
            object->OnDameged(shared_from_this(), GetSkillData().damege + GetOwner()->GetObjectStat().attack());
        });
        //소멸
        GetRoom()->DoAsync(&Room::LeaveGame, GetObjectId());
        return;
    }

    //update문이 타이머에의해 시간관리가 되지만 이부분은 캐릭터가 스킬연타시 막기위해 체크
    if (_nextMoveTick >= GetTickCount64())
        return;
    long tick = (long)(1000 / GetSkillData().projectile.speed);
    _nextMoveTick = GetTickCount64() + tick;

    Vector2Int destPos = GetFrontCellPos();
    if (GetRoom()->GetMap().CanGo(destPos, true))
    {
        SetCellPos(destPos.posx, destPos.posy);

        Protocol::S_MOVE movePacket;
        movePacket.set_objectid(GetObjectId());
        movePacket.mutable_posinfo()->set_movedir(GetMoveDir());
        movePacket.mutable_posinfo()->set_posx(GetPosx());
        movePacket.mutable_posinfo()->set_posy(GetPosy());
        movePacket.mutable_posinfo()->set_state(GetState());

        auto movePacketBuffer = ClientPacketHandler::MakeSendBuffer(movePacket);
        GetRoom()->DoAsync(&Room::Broadcast,movePacketBuffer);

        cout << "Move Arrow" << endl;
    }
    else
    {
        GameObjectRef target = GetRoom()->GetMap().Find(destPos);
        if (target != nullptr)
        {
            GetRoom()->DoAsync([this,target]() {
                target->OnDameged(shared_from_this(), GetSkillData().damege + GetOwner()->GetObjectStat().attack());
            });
        }

        //소멸
        GetRoom()->DoAsync(&Room::LeaveGame, GetObjectId());
    }
}
