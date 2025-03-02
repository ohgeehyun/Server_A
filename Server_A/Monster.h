#pragma once
#include "GameObject.h"

class Monster : public GameObject
{
public:
    Monster();
    ~Monster();

    //FSM Finite State Machine 간단한 인공지능에 적합
    void Update() override;

    void SetTarget(PlayerRef& player) { _target = player; }
    PlayerRef GetTarget() const { return _target; }

    void SetSearchCellDist(int32 value) { _searchCellDist = value; }
    int32 GetSearchCellDist() const { return _searchCellDist; }

    void SetChaseCellDist(int32 value) { _chaseCellDist = value; }
    int32 GetChaseCellDist() const { return _chaseCellDist; }

    void BroadCastMove();

    void OnDead(GameObjectRef attacker) override;

protected:
    virtual void UpdateIdle();
    virtual void UpdateMoving();
    virtual void UpdateSkill();
    virtual void UpdateDead();
private:
    long _nextSearchTick = 0;
    long _nextMoveTick = 0;
    long _coolTick = 0;

    int32 _skillRange = 1;
    int32 _searchCellDist = 7;
    int32 _chaseCellDist = 10;
    PlayerRef _target;
};

