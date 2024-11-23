#pragma once
#include "Protocol.pb.h"
#include "MapManager.h"

class GameObject
{
public:
    GameObject() { *_info.mutable_posinfo() = _posInfo; };
    virtual ~GameObject() {};

public:
    Protocol::GameObjectType GetObjecctType() { return _objectType; }
    
    RoomRef& GetRoom() { return _room; }
    void SetRoom(RoomRef room) { _room = room; }

    Protocol::OBJECT_INFO& GetObjectInfo() { return _info; }
    void SetObjectInfo(Protocol::OBJECT_INFO& info) { _info = info; }
    Protocol::POSITIONINFO GetPosInfo() { return _posInfo; };

    Vector2Int GetFrontCellPos(Protocol::MoveDir dir);

    Vector2Int GetCellPos() { return  Vector2Int(_posInfo.posx(), _posInfo.posy()); }
    void SetCellPos(int32 posx, int32 posy) { _posInfo.set_posx(posx); _posInfo.set_posy(posy); }

protected:
    void SetGameObjectType(Protocol::GameObjectType& objectType) { _objectType = objectType; }
private:
    void SetPosInfo(Protocol::POSITIONINFO& posinfo) { _posInfo = posinfo; }

private:
    Protocol::GameObjectType _objectType = Protocol::GameObjectType::NONE;
    RoomRef _room;
    Protocol::OBJECT_INFO _info;
    Protocol::POSITIONINFO _posInfo;
};

