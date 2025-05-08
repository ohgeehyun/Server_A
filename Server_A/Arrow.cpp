#include "pch.h"
#include "Arrow.h"
#include "Room.h"
#include "GameObject.h"
#include "ClientPacketHandler.h"

Arrow::Arrow()
{
    SetGameObjectType(Common::PROJECTTILE);
    //_skillData.id = 0;
}

Arrow::~Arrow()
{
    cout << GetObjectId() << " 오브젝트 소멸자 호출 완료 " << endl;
}

void Arrow::Update()
{
   
}
