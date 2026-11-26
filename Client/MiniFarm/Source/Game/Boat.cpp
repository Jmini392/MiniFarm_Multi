#include "PCH.h"
#include "Boat.h"
#include "Player.h"
#include "InputManager.h"

void Boat::OnInteract(Player* player)
{
    player->SetEscapeBoatId(m_netId);
    player->SetSysMsg("Request escape?\nYes[Y]\nNo[N]");
    player->SetEscaping(true);
    InputManager::SetUIMode(true);
}
