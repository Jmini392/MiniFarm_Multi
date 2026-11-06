#include "InventoryCore.h"
#include <limits>

namespace
{
    bool IsInventoryItem(EItemID id)
    {
        return id == EItemID::Carrot || id == EItemID::Cabbage
            || id == EItemID::SeedCarrot || id == EItemID::SeedCabbage;
    }
}

bool InventoryCore::AddItem(EItemID id, int count)
{
    if (!IsInventoryItem(id) || count <= 0)
        return false;

    const int currentCount = GetItemCount(id);
    if (count > (std::numeric_limits<int>::max)() - currentCount)
        return false;

    m_items[id] = currentCount + count;
    return true;
}

bool InventoryCore::UseItem(EItemID id, int count)
{
    if (!IsInventoryItem(id) || count <= 0)
        return false;

    const auto it = m_items.find(id);
    if (it == m_items.end() || it->second < count)
        return false;

    it->second -= count;
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
