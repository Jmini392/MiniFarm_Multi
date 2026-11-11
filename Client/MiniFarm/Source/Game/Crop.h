#pragma once
#include "InteractableObject.h"
#include "Common/CropState.h"
#include "Common/GameConfig.h"

class Crop : public InteractableObject
{
private:
    CropState m_state;
    bool m_receivedInitialState = false;
    void RefreshModelFromState();
    bool IsServerManaged() const { return GameConfig::ServerFarmingEnabled && m_netId >= 0; }
public:
    Crop(const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& scale,
        const std::string& model, const std::string& texture);

	void SetCropID(EItemID id) { if (!IsServerManaged()) m_state.id = id; }
    void SetCropState(EItemID newID, int newLevel);
    // Apply the full server state without resetting water or growth time.
    bool ApplyServerState(const CropState& state);
    void Update(int dt) override;
    void AddWater(float amount);
    virtual void OnInteract(Player* player) override;

    float GetWaterProgress() const;
    float GetGrowProgress() const;
    void DrawBar();
};
