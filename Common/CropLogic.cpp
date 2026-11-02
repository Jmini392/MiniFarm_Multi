#include "CropLogic.h"

bool CropLogic::UpdateGrowth(CropState& state, const CropData& data, int dt)
{
    if (state.id == EItemID::Empty || state.id == EItemID::Tilled)
        return false;

    int maxLevel = (int)data.stageTypes.size() - 1;
    if (state.level >= maxLevel)
        return false;

    float requiredWater = data.waterStages[state.level];

    if (state.water >= requiredWater)
    {
        state.growTime += dt / 1000.f;
    }

    bool grew = false;
    if (state.growTime >= state.requiredGrowTime)
    {
        state.water -= requiredWater;
        state.growTime = 0.f;
        state.level++;
        grew = true;
    }

    if (state.water < 0.f)
        state.water = 0.f;

    return grew;
}
