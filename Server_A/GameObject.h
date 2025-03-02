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
    Protocol::GameObjectType GetGameObjectType() const { return _objectType; }

    RoomRef GetRoom() { return _room; }
    void SetRoom(RoomRef room) { _room = room; }

    Protocol::OBJECT_INFO& GetObjectInfo() { return _info; }
    //현재는 참조로 받아서 관리하는데 문제 생길시 값 또는 수정
    void SetObjectInfo(Protocol::OBJECT_INFO& info) { _info = info; }

    Protocol::MoveDir GetMoveDir() const { return _info.posinfo().movedir(); }
    void SetMoveDir(Protocol::MoveDir movedir) { _info.mutable_posinfo()->set_movedir(movedir); }

    int32 GetPosx() const { return _info.posinfo().posx(); }
    void  SetPosx(int32 posx) { _info.mutable_posinfo()->set_posx(posx); }
   
    int32 GetPosy() const { return _info.posinfo().posy(); }
    void  SetPosy(int32 posy) { _info.mutable_posinfo()->set_posy(posy); }

    int32 GetLevel() const { return _info.statinfo().level(); }
    void  SetLevel(int32 level) { _info.mutable_statinfo()->set_level(level); }

    Protocol::CreatureState GetState()const { return _info.posinfo().state(); }
    void SetState(Protocol::CreatureState state) { _info.mutable_posinfo()->set_state(state); }

    int32 GetObjectId() const { return _info.objectid(); }
    void SetObjectId(int32 id) { _info.set_objectid(id); }

    Protocol::STATINFO GetObjectStat() const { return _info.statinfo(); }
    void SetObjectStat(Protocol::STATINFO stat) { _info.mutable_statinfo()->CopyFrom(stat); }

    int32 GetHp() const { return _info.statinfo().hp(); }
    void SetHp(int32 hp) { _info.mutable_statinfo()->set_hp(hp); }

    int32 GetMaxHp() const { return _info.statinfo().maxhp(); }
    void SetMaxHp(int32 maxHp) { _info.mutable_statinfo()->set_maxhp(maxHp); }

    float GetSpeed() { return _info.statinfo().speed(); };
    void SetSpeed(float speed) { _info.mutable_statinfo()->set_speed(speed); };

    Protocol::POSITIONINFO GetPosinfo() const { return _info.posinfo(); }

    Vector2Int GetFrontCellPos(Protocol::MoveDir dir);
    Vector2Int GetFrontCellPos();

    static Protocol::MoveDir GetDirFromVec(Vector2Int dir);


    Vector2Int GetCellPos()const { return  Vector2Int(GetPosx(), GetPosy()); }
    void SetCellPos(int32 posx, int32 posy) { _info.mutable_posinfo()->set_posx(posx); _info.mutable_posinfo()->set_posy(posy); }


public:

    virtual void OnDameged(GameObjectRef attacker, int damege);
    virtual void OnDead(GameObjectRef attacker);
    virtual void Update();
  
protected:
    //상속 받은 객체들이 생성시에 오브젝트타입을 설정
    void SetGameObjectType(Protocol::GameObjectType objectType) { _objectType = objectType; }

private:

private:
    Protocol::GameObjectType _objectType = Protocol::GameObjectType::NONE;
    RoomRef _room;
    Protocol::OBJECT_INFO _info;
};

