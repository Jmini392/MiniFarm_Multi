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

    TradeResult SellCrop(InventoryCore& inventory, EItemID cropID,
        int count, const CropData& data);
}
