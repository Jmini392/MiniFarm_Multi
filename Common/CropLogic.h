#pragma once
#include "CropState.h"

class InventoryCore;

namespace CropLogic
{
    enum class ActionResult
    {
        Success,
        NotApplicable,
        NoSeed
    };

    void ResetState(CropState& state, EItemID id, int level);
    ActionResult Till(CropState& state);
    ActionResult Plant(CropState& state, InventoryCore& inventory, EItemID seedID);
    void AddWater(CropState& state, float amount);
    ActionResult Water(CropState& state);
    ActionResult Harvest(CropState& state, InventoryCore& inventory, const CropData& data);

    // Returns true when the growth stage changes. dt is in milliseconds.
    bool UpdateGrowth(CropState& state, const CropData& data, int dt);
}
