#include "pch.h"
#include "Player.h"
#include "ServerProtocol.pb.h"
#include "ObjectManager.h"


bool ObjectManager::Remove(int32 objectId)
{
    Common::GameObjectType objectType = GetObjectTypeById(objectId);
    WRITE_LOCK
    {
        if (objectType == Common::PLAYER)
         return _objects.erase(objectId);
    }
    return false;
}

GameObjectRef ObjectManager::Find(int32 objectId)
{
    Common::GameObjectType objectType = GetObjectTypeById(objectId);
    WRITE_LOCK
    {
         if (objectType == Common::PLAYER)
         {
             GameObjectRef player = nullptr;
             for (auto it = _objects.begin(); it != _objects.end(); ++it)
             {
                 if (it->first == objectId)
                 {
                     player = it->second;
                     return player;
                 }
             }
         }
    }
    return nullptr;
}

int32 ObjectManager::GenerateId(Common::GameObjectType type)
{
    WRITE_LOCK
    {
        //counter의 경우 하위24비트만 사용
        return ((int32)type << 24) | (_counter++);
    }

}

Common::GameObjectType ObjectManager::GetObjectTypeById(int32 id)
{
    //24비트 밀어주고 0x7F범위 7비트 까지만 사용 16진수로는 0x7F임
    int32 type = (id >> 24) & 0x7F;
    return static_cast<Common::GameObjectType>(type);
}
