#pragma once
#include "InventoryCore.h"
#include "SharedWallet.h"

namespace ShopLogic
{
    struct TradeResult
    {
        bool success;
        int totalPrice;
    };

    // All players in a world pass the same wallet. The caller supplies matching
    // CropData and serializes the entire trade. Failure preserves both objects.
    TradeResult BuySeed(InventoryCore& inventory, SharedWallet& wallet, EItemID seedID,
        int count, const CropData& data);

    TradeResult SellCrop(InventoryCore& inventory, SharedWallet& wallet, EItemID cropID,
        int count, const CropData& data);
}
