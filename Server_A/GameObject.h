#pragma once
#include "Protocol.pb.h"
#include "MapManager.h"

class GameObject : public enable_shared_from_this<GameObject>
{
    //혹여나 _info초기화 문제 될시 초기화 함수 작성
public:
    GameObject();
    virtual ~GameObject() {};

public:
    Protocol::GameObjectType GetGameObjectType() { return _objectType; }
    
    RoomRef& GetRoom() { return _room; }
    void SetRoom(RoomRef room) { _room = room; }

    Protocol::OBJECT_INFO& GetObjectInfo() { return _info; }
    void SetObjectInfo(Protocol::OBJECT_INFO& info) { _info = info; }
    Protocol::MoveDir GetMoveDir() { return _info.mutable_posinfo()->movedir();}
   int32 GetPosx() { return _info.mutable_posinfo()->posx();}
   int32 GetPosy() { return _info.mutable_posinfo()->posy();}
   Protocol::CreatureState GetState() { return _info.mutable_posinfo()->state();}

    int32 GetObjectId() { return _info.objectid(); }
    void SetObjectId(int32 id) { _info.set_objectid(id); }

    Vector2Int GetFrontCellPos(Protocol::MoveDir dir);
    Vector2Int GetFrontCellPos();

    Vector2Int GetCellPos() { return  Vector2Int(GetPosx(), GetPosy()); }
    void SetCellPos(int32 posx, int32 posy) { _info.mutable_posinfo()->set_posx(posx); _info.mutable_posinfo()->set_posy(posy); }

protected:
    void SetGameObjectType(Protocol::GameObjectType objectType) { _objectType = objectType; }
private:
private:
    Protocol::GameObjectType _objectType = Protocol::GameObjectType::NONE;
    RoomRef _room;
    Protocol::OBJECT_INFO _info;
};

