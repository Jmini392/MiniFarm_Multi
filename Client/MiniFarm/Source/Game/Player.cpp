#include "PCH.H"
#include "Player.h"
#include "SceneManager.h"
#include "InputManager.h"
#include "DataTable.h"
#include "BoxCollider.h"
#include "CollisionSystem.h"
#include "IInteractable.h"
#include "Shop.h"
#include "Shader.h"

Player::Player()
{
	m_model = std::make_shared<Model>("Models/player_body.obj", "Models/player.png");
	m_limbs.emplace_back("Models/player_left_arm.obj", "Models/player.png");
	m_limbs.emplace_back("Models/player_right_arm.obj", "Models/player.png");
	m_limbs.emplace_back("Models/player_left_leg.obj", "Models/player.png");
	m_limbs.emplace_back("Models/player_right_leg.obj", "Models/player.png");
	m_limbs.emplace_back("Models/shovel.obj", "Models/farm_texture.png");
	m_limbs.emplace_back("Models/water_pot.obj", "Models/farm_texture.png");
	m_limbs.emplace_back("Models/sickle.obj", "Models/farm_texture.png");
	m_pos = { 0.f, 8.f, 0.f };
	m_rot = { 0.f, 0.f, 0.f };
	m_scale = { 1.f, 1.f, 1.f };

	m_collider = std::make_shared<BoxCollider>(
		glm::vec3(-0.5f, 0.0f, -0.5f),
		glm::vec3(0.5f, 2.0f, 0.5f)
	);

	m_alive = true;
	m_name = "player";
}

void Player::Update(int time)
{
	TryUpdateInteractTarget();
	HandleInteractInput();
	HandleEquipmentInput();
	HandleShopDirectInput();
	HandleBoatDirectInput();

	HandleRotate();
	HandleMove();
	UpdateWalkAnimations();
	UpdateToolAnimation();
	Draw();
	ApplyGravity();

	GameObject::Update(time);
	InputManager::Update();
}

void Player::HandleRotate()
{
	if (InputManager::IsUIMode())
		return;

	auto& cam = SceneManager::GetCamera();

	float rotSpeed = 0.1f;
	cam.AddYaw(InputManager::GetDeltaX() * rotSpeed);
	cam.AddPitch(-InputManager::GetDeltaY() * rotSpeed);

	m_rot.y = cam.GetFlatYaw();

	InputManager::ResetDelta();
}

void Player::HandleMove()
{
	if (InputManager::IsUIMode())
		return;

	auto& cam = SceneManager::GetCamera();

	glm::vec3 forward = cam.GetForwardFlat();
	glm::vec3 right = cam.GetRightFlat();

	glm::vec3 move(0);

	if (InputManager::IsKeyDown('w')) move += forward;
	if (InputManager::IsKeyDown('s')) move -= forward;
	if (InputManager::IsKeyDown('a')) move -= right;
	if (InputManager::IsKeyDown('d')) move += right;
	if (InputManager::IsKeyPressed(' ') && m_isGrounded)
	{
		m_verticalVelocity = m_jumpForce;
		m_isGrounded = false;
	}

	m_isWalking = glm::length(move) > 0;

	if (m_isWalking)
	{
		glm::vec3 desired = m_pos + glm::normalize(move) * m_speed;
		desired.y = m_pos.y;
		glm::vec3 newPos = CollisionSystem::TryMove(this, desired);
		m_pos.x = newPos.x;
		m_pos.z = newPos.z;

		m_collider->UpdatePos(m_pos);
	}
	cam.FollowTarget(m_pos);

	UpdateWalkAnimations();
}

void Player::HandleInteractInput()
{
	if (m_FocusedInteractable == nullptr)
		return;

	if (InputManager::IsKeyPressed('e'))
	{
		LOG_D("상호작용");
		m_FocusedInteractable->OnInteract(this);
	}
}

void Player::HandleEquipmentInput()
{
	if (InputManager::IsKeyPressed('1'))
	{
		SetEquippedTool(EItemID::Hoe);
		LOG_D("도구 변경: 괭이");
	}
	else if (InputManager::IsKeyPressed('2'))
	{
		SetEquippedTool(EItemID::WateringCan);
		LOG_D("도구 변경: 물뿌리개");
	}
	else if (InputManager::IsKeyPressed('3'))
	{
		SetEquippedTool(EItemID::Sickle);
		LOG_D("도구 변경: 낫");
	}
	else if (InputManager::IsKeyPressed('4'))
	{
		SetEquippedTool(EItemID::SeedCarrot);
		LOG_D("도구 변경: 당근 씨앗");
	}
	else if (InputManager::IsKeyPressed('5'))
	{
		SetEquippedTool(EItemID::SeedCabbage);
		LOG_D("도구 변경: 양배추 씨앗");
	}
}

void Player::HandleShopDirectInput()
{
	if (!m_CurrentShop || !IsShopping()) return;

	if (InputManager::IsKeyPressed('b'))
	{
		LOG("구매 시도");
		m_CurrentShop->Buy(this);
	}

	if (InputManager::IsKeyPressed('s'))
	{
		LOG("판매 시도");
		m_CurrentShop->Sell(this);
	}

	if (InputManager::IsKeyPressed('x'))
	{
		LOG("상점 이용 종료");
		InputManager::SetUIMode(false);
		SetShopping(false);
	}
}

void Player::HandleBoatDirectInput()
{
	if (!IsEscaping())
		return;

	if (InputManager::IsKeyPressed('y'))
	{
		LOG("탈출 시도");
		if (CanEscape())
		{
			LOG("탈출 성공!");
			m_onBoat = true;
		}
	}

	if (InputManager::IsKeyPressed('n') || InputManager::IsKeyPressed(' '))
	{
		InputManager::SetUIMode(false);
		SetEscaping(false);
	}
}

void Player::TryUpdateInteractTarget()
{
	m_FocusedInteractable = nullptr;

	const auto& objects = SceneManager::GetObjects();
	glm::vec3 playerPos = m_pos;

	float bestDistance = std::numeric_limits<float>::max();

	for (const auto& obj : objects)
	{
		if (obj.get() == this)
			continue;

		IInteractable* interactable = dynamic_cast<IInteractable*>(obj.get());
		if (!interactable || !interactable->IsInteractable())
			continue;

		float maxDistance = interactable->GetInteractDistance();

		glm::vec3 toObj = obj->m_pos - playerPos;

		float dist = glm::length(toObj);
		if (dist > maxDistance)
			continue;

		if (dist < bestDistance)
		{
			bestDistance = dist;
			m_FocusedInteractable = interactable;
		}

		if (Shop* shop = dynamic_cast<Shop*>(obj.get()))
		{
			m_CurrentShop = shop;
		}
	}
}

void Player::BuySeed(EItemID seedID, int count)
{
	const CropData* data = nullptr;
	switch (seedID)
	{
	case EItemID::SeedCarrot:
		data = DataTable::GetCrop(EItemID::Carrot);
		break;
	case EItemID::SeedCabbage:
		data = DataTable::GetCrop(EItemID::Cabbage);
		break;
	default:
		return;
	}
	int totalPrice = data->seedPrice * count;
	if (m_inventory.SpendMoney(totalPrice))
	{
		AddItem(seedID, count);
		LOG_D("%s를 %d개 구매 (-%d원) ", data->name.c_str(), count, totalPrice);
	}
	else
	{
		LOG_D("금액 부족");
	}
}

void Player::SellCrop(EItemID cropID, int count)
{
	const CropData* data = nullptr;
	switch (cropID)
	{
	case EItemID::Carrot:
		data = DataTable::GetCrop(EItemID::Carrot);
		break;
	case EItemID::Cabbage:
		data = DataTable::GetCrop(EItemID::Cabbage);
		break;
	default:
		return;
	}

	if (UseItem(cropID, count))
	{
		int totalPrice = data->sellPrice * count;
		m_inventory.AddMoney(totalPrice);
		LOG_D("%s를 %d개 판매 (+%d원) ", data->name.c_str(), count, totalPrice);
	}
	else
	{
		LOG_D("판매할 %s 없음", data->name.c_str());
	}
}

void Player::AddItem(EItemID itemID, int count)
{
	m_inventory.AddItem(itemID, count);
}

bool Player::UseItem(EItemID itemID, int count)
{
	if (m_inventory.UseItem(itemID, count))
	{
		LOG_D("아이템 사용: %d개", count);
		return true;
	}
	LOG_D("아이템 부족");
	return false;
}
void Player::AddMoney(int amount)
{
	m_inventory.AddMoney(amount);
}

void Player::Draw()
{
	GameObject::Draw();
	glm::mat4 playerTransform =
		Translate(m_pos) *
		Rotate(m_rot.y, { 0,1,0 }) *
		Rotate(m_rot.x, { 1,0,0 }) *
		Rotate(m_rot.z, { 0,0,1 }) *
		Scale(m_scale);

	auto& shader = SceneManager::GetMainShader();

	glm::vec3 jointPositions[] = {
		{ -0.3f, 1.5f, 0.0f },
		{ 0.3f, 1.5f, 0.0f },
		{ -0.15f, 0.8f, 0.0f },
		{ 0.15f, 0.8f, 0.0f },
		{ -0.3f, 0.5f, 0.2f },
		{ -0.3f, 0.5f, 0.2f },
		{ -0.3f, 0.5f, 0.2f }
	};
	float armRotX = m_limbs[1].currentRot.x;
	float radians = glm::radians(armRotX);
	float armLength = 1.f;
	glm::vec3 toolOffset = glm::vec3(0.f, -armLength * (1.0f - std::cos(radians)), -armLength * std::sin(radians));

	for (size_t i = 0; i < m_limbs.size(); ++i) {
		auto& limb = m_limbs[i];
		if (limb.model) {
			glm::mat4 limbTransform;
			if (i == 4 && m_equippedTool == EItemID::Hoe) {
				limbTransform = playerTransform *
					Translate(jointPositions[i] + toolOffset) *
					Rotate(armRotX, { 1,0,0 }) *
					Rotate(270, { 1,0,0 }) *
					Rotate(limb.currentRot.y, { 0,1,0 }) *
					Rotate(limb.currentRot.z, { 0,0,1 });
			}
			else if (i == 4 && m_equippedTool != EItemID::Hoe) {
				continue;
			}
			else if (i == 5 && m_equippedTool == EItemID::WateringCan) {
				limbTransform = playerTransform *
					Translate(jointPositions[i] + toolOffset) *
					Rotate(armRotX, { 1,0,0 }) *
					Rotate(limb.currentRot.x, { 1,0,0 }) *
					Rotate(limb.currentRot.y, { 0,1,0 }) *
					Rotate(limb.currentRot.z, { 0,0,1 });
			}
			else if (i == 5 && m_equippedTool != EItemID::WateringCan) {
				continue;
			}
			else if (i == 6 && m_equippedTool == EItemID::Sickle) {
				limbTransform = playerTransform *
					Translate(jointPositions[i] + toolOffset) *
					Rotate(armRotX, { 1,0,0 }) *
					Rotate(90, { 1,0,0 }) *
					Rotate(limb.currentRot.y, { 0,1,0 }) *
					Rotate(limb.currentRot.z, { 0,0,1 });
			}
			else if (i == 6 && m_equippedTool != EItemID::Sickle) {
				continue;
			}
			else {
				limbTransform = playerTransform *
					Translate(jointPositions[i]) *
					Rotate(limb.currentRot.x, { 1,0,0 }) *
					Rotate(limb.currentRot.y, { 0,1,0 }) *
					Rotate(limb.currentRot.z, { 0,0,1 }) *
					Translate(-jointPositions[i]);
			}

			shader.SetModel(limbTransform);
			limb.model->Draw();
		}
	}
}

void Player::UpdateWalkAnimations()
{
	if (m_isWalking) {
		m_walkAnimTime += 0.15f;

		float armSwing = std::sin(m_walkAnimTime) * 20.f;
		if (m_limbs.size() >= 2) {
			m_limbs[0].currentRot.x = armSwing;
			m_limbs[1].currentRot.x = -armSwing;
		}

		float legSwing = std::sin(m_walkAnimTime + 3.14159f) * 20.0f;
		if (m_limbs.size() >= 4) {
			m_limbs[2].currentRot.x = legSwing;
			m_limbs[3].currentRot.x = -legSwing;
		}
	}
	else {
		for (auto& limb : m_limbs) {
			limb.currentRot *= 0.92f;
		}

		m_walkAnimTime *= 0.85f;
		if (std::fabs(m_walkAnimTime) < 0.01f) {
			m_walkAnimTime = 0.0f;
		}
	}
}

void Player::ApplyGravity()
{
	if (!m_collider)
		return;

	if (!m_isGrounded)
	{
		m_verticalVelocity += m_gravity;
	}

	const float maxFallSpeed = -1.0f;
	if (m_verticalVelocity < maxFallSpeed)
		m_verticalVelocity = maxFallSpeed;

	float newY = m_pos.y + m_verticalVelocity;

	float supportY = 0.0f;
	bool hasSupportBelow = CollisionSystem::CheckSupportBelow(this, glm::vec3(m_pos.x, newY, m_pos.z), supportY);

	if (hasSupportBelow && m_verticalVelocity <= 0.0f)
	{

		m_pos.y = supportY;
		m_isGrounded = true;
		m_verticalVelocity = 0.0f;
	}
	else
	{
		m_pos.y = newY;
		m_isGrounded = false;
	}

	m_collider->UpdatePos(m_pos);
}

void Player::UpdateToolAnimation()
{
	static float time = 0.0f;
	if (m_action)
	{
		time += 0.2f;
		m_limbs[1].currentRot.x = -45.f;
		
		if (time >= 3.14159f)
		{
			m_action = false;
			time = 0.0f;
			m_limbs[1].currentRot.x = 0.0f;
		}
	}
}