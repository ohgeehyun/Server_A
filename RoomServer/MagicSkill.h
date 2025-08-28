#pragma once
#include "ProjectTile.h"
class MagicSkill : public ProjectTile
{
public:
    MagicSkill();
    ~MagicSkill();

public:
    void Update() override;
private:
    uint32 _nextMoveTick = 0;
};

