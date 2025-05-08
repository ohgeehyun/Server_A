#pragma once

class ObjectManager
{
public:
    template <typename T>
    typename std::enable_if<std::is_base_of<GameObject, T>::value, std::shared_ptr<T>>::type Add();

    bool Remove(int32 objectId);
    GameObjectRef Find(int32 objectId);
    int32 GenerateId(Common::GameObjectType);

    static Common::GameObjectType GetObjectTypeById(int32 id);

private:
    USE_LOCK;
    HashMap<int32, GameObjectRef> _objects;

    //비트 플래그를 통한 오브젝트 관리
    //[./.......][........][........][........]
    //[UNUSED(1)][TYPE(7)][ID(24)]
    int32 _counter = 1;
};

template<typename T>
typename std::enable_if<std::is_base_of<GameObject, T>::value, std::shared_ptr<T>>::type ObjectManager::Add()
{
    std::shared_ptr<T> gameObject = std::make_shared<T>();

    gameObject->SetObjectId(GenerateId(gameObject->GetGameObjectType()));

    if (gameObject->GetGameObjectType() == Common::PLAYER)
    {
        _objects[gameObject->GetObjectId()] = gameObject;
    }

    return gameObject;
}
