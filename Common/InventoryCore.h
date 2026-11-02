#pragma once
#include "GameTypes.h"
#include <unordered_map>

class InventoryCore
{
public:
    void AddItem(EItemID id, int count = 1);
    bool UseItem(EItemID id, int count = 1);
    bool HasItem(EItemID id) const;
    int GetItemCount(EItemID id) const;

    void AddMoney(int amount);
    bool SpendMoney(int amount);
    int GetMoney() const { return m_money; }

private:
    std::unordered_map<EItemID, int> m_items;
    int m_money = 0;
};
