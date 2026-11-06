#pragma once

// One instance is owned by a world, not by each player's inventory.
// The owner serializes access; this value type does not own a mutex or socket.
class SharedWallet
{
public:
    // Zero is allowed. Rejected changes leave the balance unchanged.
    bool AddMoney(int amount);
    bool SpendMoney(int amount);
    int GetMoney() const { return m_money; }

private:
    int m_money = 0;
};
