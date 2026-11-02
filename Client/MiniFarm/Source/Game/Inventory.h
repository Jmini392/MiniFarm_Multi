#pragma once
#include "GameTypes.h"
#include "Common/InventoryCore.h"

class Inventory
{
public:
    Inventory();

    void AddItem(EItemID id, int count = 1);
    bool UseItem(EItemID id, int count = 1);
    bool HasItem(EItemID id) const;

    void AddMoney(int amount);
    bool SpendMoney(int amount);
    int GetMoney() const { return m_core.GetMoney(); }
	void DrawUI() const;
private:
	InventoryCore m_core;
};
