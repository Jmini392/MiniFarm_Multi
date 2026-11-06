#include "PCH.h"
#include "DataTable.h"
#ifdef _DEBUG
#include "Common/CropTable.h"
#endif

std::unordered_map<int, CropData> DataTable::s_crops;
std::unordered_map<std::string, ObjectInfo> DataTable::m_objects;

void DataTable::Init()
{
	LOG("DataTable Init");
	LoadCrops("Data/crops.csv");
	LoadObjects("Data/object_table.csv");

#ifdef _DEBUG
    // Debug-only copies: gameplay continues to use the existing s_crops table.
    CropTable debugCropTable;
    std::string debugCropError;
    const bool debugCropLoaded = debugCropTable.Load("Data/crops.csv", debugCropError);
    const std::size_t debugCropCount = debugCropTable.GetCount();
    const CropData* debugCarrot = debugCropTable.GetCrop(EItemID::Carrot);
    const CropData* debugCabbage = debugCropTable.GetCrop(EItemID::Cabbage);

    // Breakpoint: inspect the normal CSV results here.
    LOG_D("[CropTable] loaded=%d count=%zu carrotSeed=%d cabbageSeed=%d",
        debugCropLoaded, debugCropCount,
        debugCarrot ? debugCarrot->seedPrice : -1,
        debugCabbage ? debugCabbage->seedPrice : -1);
    if (!debugCropLoaded)
        LOG_E("[CropTable] %s", debugCropError.c_str());

    if (debugCropLoaded)
    {
        // This intentionally invalid CSV can be edited to try different errors.
        CropTable debugErrorTable = debugCropTable;
        std::string debugTestError;
        const std::size_t debugCountBefore = debugErrorTable.GetCount();
        const bool debugTestLoaded = debugErrorTable.Load("Data/crops_error_test.csv", debugTestError);
        const std::size_t debugCountAfter = debugErrorTable.GetCount();
        const CropData* debugCarrotAfter = debugErrorTable.GetCrop(EItemID::Carrot);

        // Breakpoint: a failed load should leave count=4 and carrotSeed=5.
        LOG_D("[CropTable probe] loaded=%d before=%zu after=%zu carrotSeed=%d",
            debugTestLoaded, debugCountBefore, debugCountAfter,
            debugCarrotAfter ? debugCarrotAfter->seedPrice : -1);
        LOG_D("[CropTable probe] %s", debugTestError.empty() ? "CSV is valid" : debugTestError.c_str());
    }
    std::cout.flush();
#endif
}

void DataTable::LoadCrops(const std::string& path)
{
    std::ifstream file(path);
    std::string line;

    // header skip
    std::getline(file, line);

    while (std::getline(file, line))
    {
        if (line.empty()) continue;

        std::stringstream ss(line);
        CropData data;
        std::string token;

        // id
        std::getline(ss, token, ',');
        int cropId = std::stoi(token);

        // name
        std::getline(ss, data.name, ',');

        // seedPrice
        std::getline(ss, token, ',');
        data.seedPrice = std::stoi(token);

        // sellPrice
        std::getline(ss, token, ',');
        data.sellPrice = std::stoi(token);

        // growStages
        std::getline(ss, token, ',');
        {
            std::stringstream gs(token);
            std::string t;
            while (std::getline(gs, t, '|'))
                data.stageTypes.push_back(t);
        }

        // waterStages
        std::getline(ss, token, ',');
        {
            std::stringstream ws(token);
            std::string t;
            while (std::getline(ws, t, '|'))
                data.waterStages.push_back(std::stof(t));
        }

        s_crops[cropId] = data;
    }
}

const CropData* DataTable::GetCrop(EItemID id)
{
    const auto it = s_crops.find(static_cast<int>(id));
    return it != s_crops.end() ? &it->second : nullptr;
}

void DataTable::LoadObjects(const std::string& path)
{
	std::ifstream file(path);
	std::string line;

	std::getline(file, line);

	while (std::getline(file, line))
	{
		if (line.empty()) continue;

		std::stringstream ss(line);
		std::string type, modelPath, texturePath;

		std::getline(ss, type, ',');
		std::getline(ss, modelPath, ',');
		std::getline(ss, texturePath, ',');

		ObjectInfo info;
		info.modelPath = modelPath;
		info.texturePath = texturePath;

		m_objects[type] = info;
	}
}

const ObjectInfo* DataTable::GetObjectInfo(const std::string& type)
{
	auto it = m_objects.find(type);
	if (it == m_objects.end())
		return nullptr;

	return &it->second;
}