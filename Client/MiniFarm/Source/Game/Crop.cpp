#include "PCH.h"
#include "Crop.h"
#include "Common/CropLogic.h"
#include "DataTable.h"
#include "Model.h"
#include "Player.h"
#include "UIRenderer.h"
#include "TextureLoader.h"
#include "SceneManager.h"
#include "WaterParticleSystem.h"

Crop::Crop(const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& scale,
	const std::string& model, const std::string& texture)
	: InteractableObject(pos, rot, scale)
{
	m_model = ModelCache::Get(model, texture);
}

void Crop::SetCropState(EItemID newID, int newLevel)
{
	m_state.id = newID;
	m_state.level = newLevel;
	m_state.water = 0.f;
	m_state.growTime = 0.f;
	m_state.requiredGrowTime = 3.f;

	RefreshModelFromState();
}

void Crop::RefreshModelFromState()
{
	const CropData* data = DataTable::GetCrop(m_state.id);
	if (!data || data->stageTypes.empty())
		return;

	const std::string& typeName = data->stageTypes[m_state.level];
	const ObjectInfo* info = DataTable::GetObjectInfo(typeName);

	if (info)
		m_model = ModelCache::Get(info->modelPath, info->texturePath);
}

void Crop::Update(int dt)
{
    if (m_state.id == EItemID::Empty || m_state.id == EItemID::Tilled)
        return;

    const CropData* data = DataTable::GetCrop(m_state.id);
    if (!data) return;

    if (CropLogic::UpdateGrowth(m_state, *data, dt))
    {
        RefreshModelFromState();
        LOG_D("Crop grew to level %d", m_state.level);
    }
}


void Crop::AddWater(float amount)
{
	m_state.water += amount;
}

void Crop::OnInteract(Player* player)
{
	auto tool = static_cast<EItemID>(player->GetEquippedTool());

	switch (tool)
	{
	case EItemID::Hoe:
	{
		if (m_state.id == EItemID::Empty)
		{
			SetCropState(EItemID::Tilled, 0);
			LOG_D("Soil tilled");
		}
		break;
	}
	case EItemID::SeedCarrot:
	{
		if (m_state.id == EItemID::Tilled)
		{
			if (!player->UseItem(tool)) break;
			SetCropState(EItemID::Carrot, 0);
			LOG_D("Carrot planted");
		}
		break;
	}
	case EItemID::SeedCabbage:
	{
		if (m_state.id == EItemID::Tilled)
		{
			if (!player->UseItem(tool)) break;
			SetCropState(EItemID::Cabbage, 0);
			LOG_D("Cabbage planted");
		}
		break;
	}
	case EItemID::WateringCan:
	{
		if (m_state.id != EItemID::Empty && m_state.id != EItemID::Tilled)
		{
			AddWater(5.f);
			WaterParticleSystem::Watering(m_pos);
			player->m_action = true;
			LOG_D("Crop watered. Current water: %.2f", m_state.water);
		}
		break;
	}
	case EItemID::Sickle:
	{
		const CropData* data = DataTable::GetCrop(m_state.id);

		if (data && m_state.level == (int)data->stageTypes.size() - 1)
		{
			player->AddItem(m_state.id);
			SetCropState(EItemID::Tilled, 0);
			LOG_D("Crop harvested");
		}
		break;
	}
	default:
		break;
	}
}

float Crop::GetWaterProgress() const
{
	const CropData* data = DataTable::GetCrop(m_state.id);
	if (!data) return 0.f;

	float required = data->waterStages[m_state.level];
	return std::clamp(m_state.water / required, 0.f, 1.f);
}

float Crop::GetGrowProgress() const
{
	return std::clamp(m_state.growTime / m_state.requiredGrowTime, 0.f, 1.f);
}

void Crop::DrawBar()
{
	if (m_state.id == EItemID::Empty || m_state.id == EItemID::Tilled)
		return;
	
	const CropData* data = DataTable::GetCrop(m_state.id);
	if (!data) return;

	int maxLevel = (int)data->stageTypes.size() - 1;
	if (m_state.level >= maxLevel)
		return;

	float waterP = GetWaterProgress();
	float growP = GetGrowProgress();

	glm::vec3 worldPos = m_pos + glm::vec3(0, 3.0f, 0);

	glm::mat4 view = SceneManager::GetCamera().GetView();
	glm::mat4 proj = SceneManager::GetCamera().GetProj(WINDOW_W / (float)WINDOW_H);

	glm::vec4 clip = proj * view * glm::vec4(worldPos, 1.0f);
	glm::vec3 ndc = glm::vec3(clip) / clip.w;

	glm::vec2 uiPos = { ndc.x * 0.5f + 0.5f, ndc.y * 0.5f + 0.5f };

	glm::vec2 fillPos = uiPos - glm::vec2(0.05f, 0.0f);

	TextureInfo bg = TextureLoader::Load("Assets/ui_bar_bg.png");
	TextureInfo fill = (m_state.id == EItemID::Carrot)
		? TextureLoader::Load("Assets/ui_bar_carrot.png")
		: TextureLoader::Load("Assets/ui_bar_cabbage.png");

	UIRenderer::Draw(bg, fillPos, 0.1f, { 0.6f,0.6f,0.6f,1 });

	UIRenderer::DrawFill(bg, fillPos, 0.1f, waterP, { 1,1,1,1 });
	UIRenderer::DrawFill(fill, fillPos, 0.1f, growP, { 1,1,1,1 });

}
