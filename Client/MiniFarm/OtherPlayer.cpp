#include "PCH.h"
#include "OtherPlayer.h"

OtherPlayer::OtherPlayer(int playerId, SharedWallet& wallet) : Player(wallet)
{
    m_netId = playerId;
    m_name = "OtherPlayer";

    m_collider.reset();
}

void OtherPlayer::Update(int time)
{
    GameObject::Update(time);
}