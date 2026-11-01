#pragma once
#include "GameTypes.h"

class DataTable
{
public:
    static void Init();
    static const CropData* GetCrop(EItemID  id);
    static const ObjectInfo* GetObjectInfo(const std::string& type);

private:
    static void LoadCrops(const std::string& path);
	static void LoadObjects(const std::string& path);

    static std::unordered_map<int, CropData> s_crops;
    static std::unordered_map<std::string, ObjectInfo> m_objects;
};
