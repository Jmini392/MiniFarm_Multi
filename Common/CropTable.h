#pragma once

#include "GameTypes.h"
#include <cstddef>
#include <string>
#include <unordered_map>

class CropTable
{
public:
    // Loads the existing six-column crops.csv format.
    // Failure reports a reason and preserves the previously loaded table.
    bool Load(const std::string& path, std::string& error);

    // Returns nullptr for an unknown ID without inserting a new entry.
    // A successful Load() invalidates pointers into the previous table.
    const CropData* GetCrop(EItemID id) const;
    std::size_t GetCount() const { return m_crops.size(); }

private:
    std::unordered_map<int, CropData> m_crops;
};
