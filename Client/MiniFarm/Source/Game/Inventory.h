#pragma once
#include "GameTypes.h"

class Inventory
{
public:
    Inventory();

    void AddItem(EItemID id, int count = 1);
    bool UseItem(EItemID id, int count = 1);
    bool HasItem(EItemID id) const;

    void AddMoney(int amount);
    bool SpendMoney(int amount);
    int GetMoney() const { return m_money; }
	void DrawUI() const;
private:
	std::unordered_map<EItemID, int> m_items;
	int m_money = 0;
};
