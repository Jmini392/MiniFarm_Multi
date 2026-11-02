#pragma once
#include "CropState.h"

namespace CropLogic
{
    // Returns true when the growth stage changes. dt is in milliseconds.
    bool UpdateGrowth(CropState& state, const CropData& data, int dt);
}
