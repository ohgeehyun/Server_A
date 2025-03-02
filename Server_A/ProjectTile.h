#pragma once
#include "GameObject.h"
#include "DataContent.h"

class ProjectTile : public GameObject
{
public:
    ProjectTile();
    ~ProjectTile();

    virtual void Update();

    Skill GetSkillData() { return _skillData; }
    void  SetSkillData(Skill& skill) { _skillData = skill; }

    GameObjectRef& GetOwner() { return _owner; }
    void SetOwner(GameObjectRef object) { _owner = object; }

protected:
    GameObjectRef _owner;

private:
    Skill _skillData;
  
};

