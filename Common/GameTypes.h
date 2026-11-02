#pragma once

#include <string>
#include <vector>

using ItemID = int;

struct CropData {
    std::string name;
    int seedPrice;
    int sellPrice;
    std::vector<std::string> stageTypes;
    std::vector<float> waterStages;
};

enum class EItemID : ItemID
{
    Empty = -1,
    Tilled = 0, // Tilled soil
    Carrot = 1,
    Cabbage = 2,

    // Tools
    Hoe = 3,
    WateringCan,
    Sickle,

    // Seeds
    SeedCarrot,
    SeedCabbage,
};
