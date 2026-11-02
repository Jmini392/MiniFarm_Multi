#pragma once
#include "GameTypes.h"

struct CropState
{
    EItemID id = EItemID::Empty;
    int level = 0;
    float water = 0.f;
    float growTime = 0.f;
    float requiredGrowTime = 3.f;
};
