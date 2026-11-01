#include "PCH.h"
#include "DataTable.h"

std::unordered_map<int, CropData> DataTable::s_crops;
std::unordered_map<std::string, ObjectInfo> DataTable::m_objects;

void DataTable::Init()
{
	LOG("DataTable Init");
	LoadCrops("Data/crops.csv");
	LoadObjects("Data/object_table.csv");
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

const CropData* DataTable::GetCrop(EItemID  id) { return &s_crops[(int)id]; }

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