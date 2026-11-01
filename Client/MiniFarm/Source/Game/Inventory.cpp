#include "PCH.h"
#include "Inventory.h"
#include "UIRenderer.h"
#include "TextRenderer.h"

Inventory::Inventory()
{
	//for test
	AddItem(EItemID::SeedCarrot, 1);
	AddItem(EItemID::SeedCabbage, 1);
	AddItem(EItemID::Carrot, 1);
	AddItem(EItemID::Cabbage, 1);
	m_money = 0;
}

void Inventory::AddItem(EItemID id, int count)
{
	m_items[id] += count;
}

bool Inventory::UseItem(EItemID id, int count)
{
	if (!HasItem(id))
		return false;

	m_items[id] -= count;
	return true;
}

bool Inventory::HasItem(EItemID id) const
{
	return m_items.find(id) != m_items.end() && m_items.at(id) > 0;
}

void Inventory::AddMoney(int amount)
{
	m_money += amount;
}

bool Inventory::SpendMoney(int amount)
{
	if (m_money < amount) return false;
	m_money -= amount;
	return true;
}

void Inventory::DrawUI() const
{
	glm::vec3 tcolor{ 0.f };
	UIRenderer::DrawCenter(TextureLoader::Load("Assets/ui_inventory.png"), { 0.5f,0.1f }, 0.85f);
	UIRenderer::Draw(TextureLoader::Load("Assets/ui_goldbar.png"), { 0.01f,0.9f }, 0.25f);

	std::string moneyStr = std::to_string(m_money);
	TextRenderer::Draw(moneyStr, 80, WINDOW_H - 35, 2, tcolor);

	std::string cseedscnt = "x" + std::to_string(m_items.count(EItemID::SeedCarrot) ? m_items.at(EItemID::SeedCarrot) : 0);
	std::string cabseedscnt = "x" + std::to_string(m_items.count(EItemID::SeedCabbage) ? m_items.at(EItemID::SeedCabbage) : 0);
	std::string carrotcnt = "x" + std::to_string(m_items.count(EItemID::Carrot) ? m_items.at(EItemID::Carrot) : 0);
	std::string cabbagecnt = "x" + std::to_string(m_items.count(EItemID::Cabbage) ? m_items.at(EItemID::Cabbage) : 0);

	float slotDist = 60.f;
	TextRenderer::DrawLeft(cseedscnt, WINDOW_W / 2, 20, 1.5f, tcolor);
	TextRenderer::DrawLeft(cabseedscnt, WINDOW_W / 2 + slotDist, 20, 1.5f, tcolor);
	TextRenderer::DrawLeft(carrotcnt, WINDOW_W / 2 + slotDist * 2, 20, 1.5f, tcolor);
	TextRenderer::DrawLeft(cabbagecnt, WINDOW_W / 2 + slotDist * 3, 20, 1.5f, tcolor);
}
