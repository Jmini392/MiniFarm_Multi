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
