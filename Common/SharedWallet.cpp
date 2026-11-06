#include "SharedWallet.h"
#include <limits>

bool SharedWallet::AddMoney(int amount)
{
    if (amount < 0 || amount > (std::numeric_limits<int>::max)() - m_money)
        return false;

    m_money += amount;
    return true;
}

bool SharedWallet::SpendMoney(int amount)
{
    if (amount < 0 || amount > m_money)
        return false;

    m_money -= amount;
    return true;
}
