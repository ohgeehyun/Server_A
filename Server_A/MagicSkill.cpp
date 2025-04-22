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
    
}
