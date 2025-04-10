#pragma once
#include "ProjectTile.h"

class Arrow : public ProjectTile
{
public:
    Arrow();
    ~Arrow();
public:
    void Update() override;
private:
    uint32 _nextMoveTick = 0;
};

