#include "CropLogic.h"
#include "InventoryCore.h"

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

void CropLogic::ResetState(CropState& state, EItemID id, int level)
{
    state.id = id;
    state.level = level;
    state.water = 0.f;
    state.growTime = 0.f;
    state.requiredGrowTime = 3.f;
}

CropLogic::ActionResult CropLogic::Till(CropState& state)
{
    if (state.id != EItemID::Empty)
        return ActionResult::NotApplicable;

    ResetState(state, EItemID::Tilled, 0);
    return ActionResult::Success;
}

CropLogic::ActionResult CropLogic::Plant(CropState& state,
    InventoryCore& inventory, EItemID seedID)
{
    if (state.id != EItemID::Tilled)
        return ActionResult::NotApplicable;

    EItemID cropID;
    switch (seedID)
    {
    case EItemID::SeedCarrot:
        cropID = EItemID::Carrot;
        break;
    case EItemID::SeedCabbage:
        cropID = EItemID::Cabbage;
        break;
    default:
        return ActionResult::NotApplicable;
    }

    if (!inventory.UseItem(seedID))
        return ActionResult::NoSeed;

    ResetState(state, cropID, 0);
    return ActionResult::Success;
}

void CropLogic::AddWater(CropState& state, float amount)
{
    state.water += amount;
}

CropLogic::ActionResult CropLogic::Water(CropState& state)
{
    if (state.id == EItemID::Empty || state.id == EItemID::Tilled)
        return ActionResult::NotApplicable;

    AddWater(state, 5.f);
    return ActionResult::Success;
}

CropLogic::ActionResult CropLogic::Harvest(CropState& state,
    InventoryCore& inventory, const CropData& data)
{
    if (state.id == EItemID::Empty || state.id == EItemID::Tilled)
        return ActionResult::NotApplicable;
    if (state.id != EItemID::Carrot && state.id != EItemID::Cabbage)
        return ActionResult::InvalidState;

    if (data.stageTypes.empty() || data.waterStages.size() != data.stageTypes.size())
        return ActionResult::InvalidData;
    for (const auto& stage : data.stageTypes)
    {
        if (stage.empty())
            return ActionResult::InvalidData;
    }

    if (state.level < 0 || static_cast<std::size_t>(state.level) >= data.stageTypes.size())
        return ActionResult::InvalidState;
    if (static_cast<std::size_t>(state.level) != data.stageTypes.size() - 1)
        return ActionResult::NotMature;

    if (!inventory.AddItem(state.id))
        return ActionResult::InventoryLimitExceeded;
    ResetState(state, EItemID::Tilled, 0);
    return ActionResult::Success;
}
