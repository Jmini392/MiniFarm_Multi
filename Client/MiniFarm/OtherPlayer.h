#pragma once

#include "Player.h"

class OtherPlayer : public Player
{
public:
    OtherPlayer(int playerId, SharedWallet& wallet);

    void Update(int time) override;
};