#pragma once
#include "InventoryCore.h"

namespace ShopLogic
{
    struct TradeResult
    {
        bool success;
        int totalPrice;
    };

    TradeResult BuySeed(InventoryCore& inventory, EItemID seedID,
        int count, const CropData& data);
}
