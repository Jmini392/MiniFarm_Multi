#include "ShopLogic.h"
#include <cstdint>
#include <limits>
#include <utility>

namespace
{
    ShopLogic::TradeFailure CalculatePrice(int unitPrice, int count, int& totalPrice)
    {
        if (count <= 0)
            return ShopLogic::TradeFailure::InvalidCount;
        if (unitPrice < 0)
            return ShopLogic::TradeFailure::InvalidPrice;

        const std::int64_t total = static_cast<std::int64_t>(unitPrice) * count;
        if (total > (std::numeric_limits<int>::max)())
            return ShopLogic::TradeFailure::PriceOverflow;

        totalPrice = static_cast<int>(total);
        return ShopLogic::TradeFailure::None;
    }
}

ShopLogic::TradeResult ShopLogic::BuySeed(InventoryCore& inventory, SharedWallet& wallet,
    EItemID seedID, int count, const CropData& data)
{
    int totalPrice = 0;
    if (seedID != EItemID::SeedCarrot && seedID != EItemID::SeedCabbage)
        return { false, 0, TradeFailure::InvalidItem };

    const TradeFailure priceFailure = CalculatePrice(data.seedPrice, count, totalPrice);
    if (priceFailure != TradeFailure::None)
        return { false, 0, priceFailure };

    InventoryCore nextInventory = inventory;
    SharedWallet nextWallet = wallet;
    if (!nextWallet.SpendMoney(totalPrice))
        return { false, totalPrice, TradeFailure::InsufficientMoney };
    if (!nextInventory.AddItem(seedID, count))
        return { false, totalPrice, TradeFailure::ItemLimitExceeded };

    inventory = std::move(nextInventory);
    wallet = nextWallet;
    return { true, totalPrice, TradeFailure::None };
}

ShopLogic::TradeResult ShopLogic::SellCrop(InventoryCore& inventory, SharedWallet& wallet,
    EItemID cropID, int count, const CropData& data)
{
    int totalPrice = 0;
    if (cropID != EItemID::Carrot && cropID != EItemID::Cabbage)
        return { false, 0, TradeFailure::InvalidItem };

    const TradeFailure priceFailure = CalculatePrice(data.sellPrice, count, totalPrice);
    if (priceFailure != TradeFailure::None)
        return { false, 0, priceFailure };

    InventoryCore nextInventory = inventory;
    SharedWallet nextWallet = wallet;
    if (!nextInventory.UseItem(cropID, count))
        return { false, 0, TradeFailure::InsufficientItems };
    if (!nextWallet.AddMoney(totalPrice))
        return { false, 0, TradeFailure::MoneyLimitExceeded };

    inventory = std::move(nextInventory);
    wallet = nextWallet;
    return { true, totalPrice, TradeFailure::None };
}
