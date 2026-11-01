#pragma once
#include "GameObject.h"
#include "Inventory.h"

class IInteractable;
class Text;
class Shop;
struct Limb {
    std::shared_ptr<Model> model;
    glm::vec3 currentRot;
    
    Limb(const std::string& objPath, const std::string& imgPath)
        : currentRot(0.f) { model = std::make_shared<Model>(objPath, imgPath); }
};

class Player : public GameObject
{
public:
    Player();
    virtual void Update(int time);
    virtual void Draw();
private:
    void HandleRotate();
    void HandleMove();
    void HandleInteractInput();
    void TryUpdateInteractTarget();
    
    void HandleEquipmentInput();
    void HandleShopDirectInput();
    void HandleBoatDirectInput();

    void UpdateWalkAnimations();
	void UpdateToolAnimation();
    void ApplyGravity();
public:
    void BuySeed(EItemID seedID, int count = 1);
    void SellCrop(EItemID cropID, int count = 1);
    void AddItem(EItemID itemID, int count = 1);
    bool UseItem(EItemID itemID, int count = 1);
    void AddMoney(int amount);

    bool HasInteractTarget() { return m_FocusedInteractable != nullptr; }
    EItemID GetEquippedTool() const { return m_equippedTool; }
    void SetEquippedTool(EItemID tool) { m_equippedTool = tool; }
    Inventory& GetInventory() { return m_inventory; }

    bool IsShopping() const { return m_shopping; }
    void SetShopping(bool shopping) { m_shopping = shopping; }
    bool IsEscaping() const { return m_escaping; }
    void SetEscaping(bool escaping) { m_escaping = escaping; }
    void SetSysMsg(const std::string& msg) { m_sysmsg = msg; }
    std::string GetSysMsg() const { return m_sysmsg; }
    void SetCanEscape(bool canescape) { m_canescape = canescape; }
    bool CanEscape() const { return m_canescape; }
    bool IsOnBoat() const { return m_onBoat; }

    bool m_action = false;
private:
    float m_speed = 0.2f;
    Inventory m_inventory;
    IInteractable* m_FocusedInteractable = nullptr;
    EItemID m_equippedTool = EItemID::Empty;
    Shop* m_CurrentShop = nullptr;
    bool m_shopping = false;
    bool m_escaping = false;
    bool m_canescape = false;
    bool m_onBoat = false;

    std::string m_sysmsg = "";

    std::vector<Limb> m_limbs;
    float m_walkAnimTime = 0.0f;
    bool m_isWalking = false;
    float m_verticalVelocity = 0.0f;
    float m_gravity = -0.035f;
    float m_jumpForce = 0.4f;
    bool m_isGrounded = true;
};
