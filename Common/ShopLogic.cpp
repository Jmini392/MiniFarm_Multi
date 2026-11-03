#include "ShopLogic.h"

ShopLogic::TradeResult ShopLogic::BuySeed(InventoryCore& inventory,
    EItemID seedID, int count, const CropData& data)
{
    int totalPrice = data.seedPrice * count;
    if (!inventory.SpendMoney(totalPrice))
        return { false, totalPrice };

    inventory.AddItem(seedID, count);
    return { true, totalPrice };
}

ShopLogic::TradeResult ShopLogic::SellCrop(InventoryCore& inventory,
    EItemID cropID, int count, const CropData& data)
{
    if (!inventory.UseItem(cropID, count))
        return { false, 0 };

    int totalPrice = data.sellPrice * count;
    inventory.AddMoney(totalPrice);
    return { true, totalPrice };
}
