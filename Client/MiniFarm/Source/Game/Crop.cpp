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


#ifdef _DEBUG
namespace
{
    const char* HarvestFailureMessage(CropLogic::ActionResult result)
    {
        switch (result)
        {
        case CropLogic::ActionResult::NotApplicable: return "No crop to harvest.";
        case CropLogic::ActionResult::NotMature: return "Crop is not mature.";
        case CropLogic::ActionResult::InventoryLimitExceeded: return "Item limit reached.";
        case CropLogic::ActionResult::InvalidState: return "Invalid crop state.";
        case CropLogic::ActionResult::InvalidData: return "Invalid crop data.";
        default: return "Harvest rejected.";
        }
    }
}

#endif

Crop::Crop(const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& scale,
	const std::string& model, const std::string& texture)
	: InteractableObject(pos, rot, scale)
{
	m_model = ModelCache::Get(model, texture);
}

void Crop::SetCropState(EItemID newID, int newLevel)
{
    // 서버가 관리하는 밭은 수신한 상태로만 바뀐다.
    if (IsServerManaged())
        return;

    CropLogic::ResetState(m_state, newID, newLevel);
    RefreshModelFromState();
}

bool Crop::ApplyServerState(const CropState& state)
{
    // 작물 종류와 모델 범위를 확인한 뒤 적용해야 잘못된 인덱스 접근을 막을 수 있다.
    if (state.id != EItemID::Empty && state.id != EItemID::Tilled
        && state.id != EItemID::Carrot && state.id != EItemID::Cabbage)
        return false;

    const CropData* data = DataTable::GetCrop(state.id);
    if (data == nullptr || state.level < 0
        || static_cast<std::size_t>(state.level) >= data->stageTypes.size()
        || static_cast<std::size_t>(state.level) >= data->waterStages.size())
        return false;

    if (!std::isfinite(state.water) || state.water < 0.f
        || !std::isfinite(state.growTime) || state.growTime < 0.f
        || !std::isfinite(state.requiredGrowTime) || state.requiredGrowTime <= 0.f)
        return false;

    if (DataTable::GetObjectInfo(data->stageTypes[state.level]) == nullptr)
        return false;

    // 로컬 농사 중에는 첫 접속 상태만 적용하고 이후 서버 갱신은 덮어쓰지 않는다.
    if (!GameConfig::ServerFarmingEnabled && m_netId >= 0 && m_receivedInitialState)
        return true;

    m_receivedInitialState = true;
    const bool modelChanged = m_state.id != state.id || m_state.level != state.level;
    m_state = state; // ResetState를 사용하면 수신한 물과 성장 시간이 사라진다.
    if (modelChanged)
        RefreshModelFromState();
    return true;
}

void Crop::RefreshModelFromState()
{
	const CropData* data = DataTable::GetCrop(m_state.id);
	if (!data || m_state.level < 0
        || static_cast<std::size_t>(m_state.level) >= data->stageTypes.size())
		return;

	const std::string& typeName = data->stageTypes[m_state.level];
	const ObjectInfo* info = DataTable::GetObjectInfo(typeName);

	if (info)
		m_model = ModelCache::Get(info->modelPath, info->texturePath);
}

void Crop::Update(int dt)
{
    // 서버가 관리하는 밭은 수신한 상태로만 바뀐다.
    if (IsServerManaged())
        return;

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
    // 서버가 관리하는 밭은 수신한 상태로만 바뀐다.
    if (IsServerManaged())
        return;

	CropLogic::AddWater(m_state, amount);
}

void Crop::OnInteract(Player* player)
{
    // 서버가 관리하는 밭은 수신한 상태로만 바뀐다.
    if (IsServerManaged())
        return;

	auto tool = static_cast<EItemID>(player->GetEquippedTool());

	switch (tool)
	{
	case EItemID::Hoe:
	{
		if (CropLogic::Till(m_state) == CropLogic::ActionResult::Success)
		{
			RefreshModelFromState();
			LOG_D("Soil tilled");
		}
		break;
	}
	case EItemID::SeedCarrot:
	{
		auto result = CropLogic::Plant(m_state, player->GetInventory().Core(), tool);
		if (result == CropLogic::ActionResult::Success)
		{
			LOG_D("아이템 사용: %d개", 1);
			RefreshModelFromState();
			LOG_D("Carrot planted");
		}
		else if (result == CropLogic::ActionResult::NoSeed)
		{
			LOG_D("아이템 부족");
		}
		break;
	}
	case EItemID::SeedCabbage:
	{
		auto result = CropLogic::Plant(m_state, player->GetInventory().Core(), tool);
		if (result == CropLogic::ActionResult::Success)
		{
			LOG_D("아이템 사용: %d개", 1);
			RefreshModelFromState();
			LOG_D("Cabbage planted");
		}
		else if (result == CropLogic::ActionResult::NoSeed)
		{
			LOG_D("아이템 부족");
		}
		break;
	}
	case EItemID::WateringCan:
	{
		auto result = CropLogic::Water(m_state);
		if (result == CropLogic::ActionResult::Success)
		{
			WaterParticleSystem::Watering(m_pos);
			player->m_action = true;
			LOG_D("Crop watered. Current water: %.2f", m_state.water);
		}
		break;
	}
	case EItemID::Sickle:
	{
		const CropData* data = DataTable::GetCrop(m_state.id);

		const CropData missingData{};
		const auto result = CropLogic::Harvest(m_state, player->GetInventory().Core(),
			data ? *data : missingData);
		if (result == CropLogic::ActionResult::Success)
		{
			RefreshModelFromState();
			LOG_D("Crop harvested");
		}
		else
		{
			LOG_D("Harvest failed: %s", HarvestFailureMessage(result));
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
