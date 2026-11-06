#pragma once
#include "GameTypes.h"
#include <unordered_map>

class InventoryCore
{
public:
    // Crop items and seeds only. A rejected change leaves quantities unchanged.
    bool AddItem(EItemID id, int count = 1);
    bool UseItem(EItemID id, int count = 1);
    bool HasItem(EItemID id) const;
    int GetItemCount(EItemID id) const;

private:
    std::unordered_map<EItemID, int> m_items;
};
