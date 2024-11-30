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
    
    Protocol::MoveDir GetMoveDir() { return _info.posinfo().movedir();}
    void SetMoveDir(Protocol::MoveDir movedir) { _info.mutable_posinfo()->set_movedir(movedir); }
    
    int32 GetPosx() { return _info.posinfo().posx();}
    void  SetPosx(int32 posx) { _info.mutable_posinfo()->set_posx(posx); }
    int32 GetPosy() { return _info.posinfo().posy(); }
    void  SetPosy(int32 posy) { _info.mutable_posinfo()->set_posy(posy); }
    
    Protocol::CreatureState GetState() { return _info.posinfo().state();}
    void SetState(Protocol::CreatureState state) { _info.mutable_posinfo()->set_state(state); }

    int32 GetObjectId() { return _info.objectid(); }
    void SetObjectId(int32 id) { _info.set_objectid(id); }

    Protocol::STATINFO GetObjectStat() { return _info.statinfo(); }
    void SetObjectStat(Protocol::STATINFO stat) { _info.mutable_statinfo()->CopyFrom(stat);}

    int32 GetHp() const { return _info.statinfo().hp(); }
    void SetHp(int32 hp) { _info.mutable_statinfo()->set_hp(hp); }

    float GetSpped() { return _info.statinfo().speed();};
    void SetSpeed(float speed) { _info.mutable_statinfo()->set_speed(speed);};

    Vector2Int GetFrontCellPos(Protocol::MoveDir dir);
    Vector2Int GetFrontCellPos();

    Vector2Int GetCellPos() { return  Vector2Int(GetPosx(), GetPosy()); }
    void SetCellPos(int32 posx, int32 posy) { _info.mutable_posinfo()->set_posx(posx); _info.mutable_posinfo()->set_posy(posy); }

    virtual void OnDameged(GameObjectRef attacker, int damage);
    virtual void OnDead(GameObjectRef attacker);
protected:
    void SetGameObjectType(Protocol::GameObjectType objectType) { _objectType = objectType; }
private:
private:
    Protocol::GameObjectType _objectType = Protocol::GameObjectType::NONE;
    RoomRef _room;
    Protocol::OBJECT_INFO _info;
};

