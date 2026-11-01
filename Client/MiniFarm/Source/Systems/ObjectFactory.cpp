#include "PCH.h"
#include "ObjectFactory.h"
#include "StaticProp.h"
#include "Crop.h"
#include "Shop.h"
#include "Boat.h"
#include "DataTable.h"
#include "SceneManager.h"
#include "IngameScene.h"

std::shared_ptr<GameObject> ObjectFactory::Create(
    const std::string& type,
    const glm::vec3& pos,
    const glm::vec3& rot,
    const glm::vec3& scale,
    const ObjectInfo* info
)
{
    if (type == "Field")
    {
        return std::make_shared<Crop>(pos, rot, scale, info->modelPath, info->texturePath);
    }
    else if (type == "House")
    {
        static int hcnt = 0;
        auto cropType = (hcnt == 0) ? EItemID::Carrot : EItemID::Cabbage;
        Scene* curScene = SceneManager::GetScene();
        if(dynamic_cast<IngameScene*>(curScene))
            hcnt++;

        auto shop = std::make_shared<Shop>(pos, rot, scale, info->modelPath, info->texturePath);
        shop->SetCropType(cropType);
        return shop;
    }
    else if (type == "Boat")
    {
        return std::make_shared<Boat>(pos, rot, scale, info->modelPath, info->texturePath);
    }

    return std::make_shared<StaticProp>(pos, rot, scale, info->modelPath, info->texturePath);
}

