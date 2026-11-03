#include "InventoryCore.h"

void InventoryCore::AddItem(EItemID id, int count)
{
    m_items[id] += count;
}

bool InventoryCore::UseItem(EItemID id, int count)
{
    if (!HasItem(id))
        return false;

    m_items[id] -= count;
    return true;
}

bool InventoryCore::HasItem(EItemID id) const
{
    return GetItemCount(id) > 0;
}

int InventoryCore::GetItemCount(EItemID id) const
{
    const auto it = m_items.find(id);
    return it != m_items.end() ? it->second : 0;
}

void InventoryCore::AddMoney(int amount)
{
    m_money += amount;
}

bool InventoryCore::SpendMoney(int amount)
{
    if (m_money < amount) return false;
    m_money -= amount;
    return true;
}
