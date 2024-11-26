#pragma once
#include "ProjectTile.h"
class Arrow : public ProjectTile
{
public:
    Arrow();
    ~Arrow();
public:
    void Update() override;
    
    GameObjectRef& GetOwner() { return _owner; }
    void SetOwner(GameObjectRef object) { _owner = object; }


private:
    GameObjectRef _owner;
    uint32 _nextMoveTick = 0;
};

