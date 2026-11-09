#include "PCH.h"
#include "Inventory.h"
#include "UIRenderer.h"
#include "TextRenderer.h"

Inventory::Inventory() = default;

bool Inventory::AddItem(EItemID id, int count)
{
    return m_core.AddItem(id, count);
}

bool Inventory::UseItem(EItemID id, int count)
{
    return m_core.UseItem(id, count);
}

bool Inventory::HasItem(EItemID id) const
{
    return m_core.HasItem(id);
}

void Inventory::DrawUI() const
{
	glm::vec3 tcolor{ 0.f };
	UIRenderer::DrawCenter(TextureLoader::Load("Assets/ui_inventory.png"), { 0.5f,0.1f }, 0.85f);

	std::string cseedscnt = "x" + std::to_string(m_core.GetItemCount(EItemID::SeedCarrot));
	std::string cabseedscnt = "x" + std::to_string(m_core.GetItemCount(EItemID::SeedCabbage));
	std::string carrotcnt = "x" + std::to_string(m_core.GetItemCount(EItemID::Carrot));
	std::string cabbagecnt = "x" + std::to_string(m_core.GetItemCount(EItemID::Cabbage));

	float slotDist = 60.f;
	TextRenderer::DrawLeft(cseedscnt, WINDOW_W / 2, 20, 1.5f, tcolor);
	TextRenderer::DrawLeft(cabseedscnt, WINDOW_W / 2 + slotDist, 20, 1.5f, tcolor);
	TextRenderer::DrawLeft(carrotcnt, WINDOW_W / 2 + slotDist * 2, 20, 1.5f, tcolor);
	TextRenderer::DrawLeft(cabbagecnt, WINDOW_W / 2 + slotDist * 3, 20, 1.5f, tcolor);
}
