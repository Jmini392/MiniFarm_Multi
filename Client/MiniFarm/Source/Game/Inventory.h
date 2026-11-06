#pragma once
#include "GameTypes.h"
#include "Common/InventoryCore.h"

class Inventory
{
public:
    Inventory();

    bool AddItem(EItemID id, int count = 1);
    bool UseItem(EItemID id, int count = 1);
    bool HasItem(EItemID id) const;

	void DrawUI() const;

    InventoryCore& Core() { return m_core; }
    const InventoryCore& Core() const { return m_core; }
private:
	InventoryCore m_core;
};
