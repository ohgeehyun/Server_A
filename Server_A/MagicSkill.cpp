#include "pch.h"
#include "MagicSkill.h"
#include "Room.h"
#include "GameObject.h"
#include "ClientPacketHandler.h"

MagicSkill::MagicSkill()
{
    SetGameObjectType(Protocol::MAGIC);
    //_skillData.id = 0;
}

MagicSkill::~MagicSkill()
{
    cout << GetObjectId() << " 아이디 마법 오브젝트 소멸자 호출 완료 " << endl;
}

void MagicSkill::Update()
{
    if (GetSkillData().id == 0 || _owner == nullptr || GetRoom() == nullptr)
        return;


    GameObjectRef object = GetRoom()->GetMap().Find(Vector2Int(GetPosx(), GetPosy()));

    if (object != nullptr)
    {
        if (GetObjectId() != object->GetObjectId())
        {
            GetRoom()->DoAsync([this, object]() {
                object->OnDameged(GetOwner(), GetSkillData().damege + GetOwner()->GetObjectStat().attack());
            });
        }
        //소멸
        GetRoom()->DoTimer(500,&Room::LeaveGame, GetObjectId());
        return;
    }
}
