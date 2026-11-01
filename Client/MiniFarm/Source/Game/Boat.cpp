#include "PCH.h"
#include "Boat.h"
#include "Player.h"
#include "InputManager.h"

void Boat::OnInteract(Player* player)
{
	int money = player->GetInventory().GetMoney();
	bool canEscape = money >= GOAL_MONEY;

	auto msg = canEscape ? "Are you sure?\nYes[Y]\nNo[N]"
		: std::format("Not enough money\nto escape!\n need {}G.\n\nOK[space]", GOAL_MONEY - money);
	
	InputManager::SetUIMode(true);
	player->SetSysMsg(msg);
	player->SetEscaping(true);
	player->SetCanEscape(canEscape);
}
