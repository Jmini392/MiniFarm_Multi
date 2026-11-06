#include "ShopLogic.h"
#include <cstdint>
#include <limits>
#include <utility>

namespace
{
    bool CalculatePrice(int unitPrice, int count, int& totalPrice)
    {
        if (unitPrice < 0 || count <= 0)
            return false;

        const std::int64_t total = static_cast<std::int64_t>(unitPrice) * count;
        if (total > (std::numeric_limits<int>::max)())
            return false;

        totalPrice = static_cast<int>(total);
        return true;
    }
}

ShopLogic::TradeResult ShopLogic::BuySeed(InventoryCore& inventory, SharedWallet& wallet,
    EItemID seedID, int count, const CropData& data)
{
    int totalPrice = 0;
    if ((seedID != EItemID::SeedCarrot && seedID != EItemID::SeedCabbage)
        || !CalculatePrice(data.seedPrice, count, totalPrice))
        return { false, 0 };

    InventoryCore nextInventory = inventory;
    SharedWallet nextWallet = wallet;
    if (!nextWallet.SpendMoney(totalPrice) || !nextInventory.AddItem(seedID, count))
        return { false, totalPrice };

    inventory = std::move(nextInventory);
    wallet = nextWallet;
    return { true, totalPrice };
}

ShopLogic::TradeResult ShopLogic::SellCrop(InventoryCore& inventory, SharedWallet& wallet,
    EItemID cropID, int count, const CropData& data)
{
    int totalPrice = 0;
    if ((cropID != EItemID::Carrot && cropID != EItemID::Cabbage)
        || !CalculatePrice(data.sellPrice, count, totalPrice))
        return { false, 0 };

    InventoryCore nextInventory = inventory;
    SharedWallet nextWallet = wallet;
    if (!nextInventory.UseItem(cropID, count) || !nextWallet.AddMoney(totalPrice))
        return { false, 0 };

    inventory = std::move(nextInventory);
    wallet = nextWallet;
    return { true, totalPrice };
}
