#include "CropTable.h"

#include <cmath>
#include <exception>
#include <fstream>
#include <sstream>
#include <utility>
#include <vector>

namespace
{
    std::string Trim(const std::string& text)
    {
        const auto first = text.find_first_not_of(" \t\r\n");
        if (first == std::string::npos)
            return {};

        const auto last = text.find_last_not_of(" \t\r\n");
        return text.substr(first, last - first + 1);
    }

    std::vector<std::string> Split(const std::string& text, char delimiter)
    {
        std::vector<std::string> values;
        std::stringstream stream(text);
        std::string value;
        while (std::getline(stream, value, delimiter))
            values.push_back(Trim(value));

        // Keep a trailing empty field so that malformed rows are rejected.
        if (text.empty() || text.back() == delimiter)
            values.emplace_back();

        return values;
    }

    bool ParseInt(const std::string& text, int& value)
    {
        try
        {
            std::size_t consumed = 0;
            value = std::stoi(text, &consumed);
            return consumed == text.size();
        }
        catch (const std::exception&)
        {
            return false;
        }
    }

    bool ParseWater(const std::string& text, float& value)
    {
        try
        {
            std::size_t consumed = 0;
            value = std::stof(text, &consumed);
            return consumed == text.size() && std::isfinite(value) && value >= 0.f;
        }
        catch (const std::exception&)
        {
            return false;
        }
    }
}

bool CropTable::Load(const std::string& path, std::string& error)
{
    error.clear();
    std::ifstream file(path);
    if (!file.is_open())
    {
        error = path + ": file - cannot open crops CSV (check path and permissions)";
        return false;
    }

    std::size_t lineNumber = 1;
    const auto fail = [&](const std::string& field, const std::string& value,
        const std::string& reason)
    {
        error = path + ":" + std::to_string(lineNumber) + ": " + field
            + " value '" + value + "' - " + reason;
        return false;
    };

    std::string line;
    if (!std::getline(file, line))
        return fail("header", "", "missing CSV header");

    // Some editors save UTF-8 CSV files with a byte-order mark.
    if (line.compare(0, 3, "\xEF\xBB\xBF") == 0)
        line.erase(0, 3);

    const std::vector<std::string> expectedHeader = {
        "id", "name", "seedPrice", "sellPrice", "growStages", "waterStages"
    };
    if (Split(line, ',') != expectedHeader)
        return fail("header", Trim(line),
            "expected id,name,seedPrice,sellPrice,growStages,waterStages");

    // Build a temporary table; publish it only after every row is valid.
    std::unordered_map<int, CropData> loadedCrops;
    while (std::getline(file, line))
    {
        ++lineNumber;
        if (Trim(line).empty())
            continue;

        const auto columns = Split(line, ',');
        if (columns.size() != expectedHeader.size())
            return fail("columns", std::to_string(columns.size()), "expected exactly 6 columns");

        int cropId = 0;
        if (!ParseInt(columns[0], cropId))
            return fail("id", columns[0], "expected an integer");
        if (loadedCrops.find(cropId) != loadedCrops.end())
            return fail("id", columns[0], "duplicate crop id");

        CropData data{};
        data.name = columns[1];
        if (data.name.empty())
            return fail("name", columns[1], "expected a non-empty name");
        if (!ParseInt(columns[2], data.seedPrice) || data.seedPrice < 0)
            return fail("seedPrice", columns[2], "expected a non-negative integer");
        if (!ParseInt(columns[3], data.sellPrice) || data.sellPrice < 0)
            return fail("sellPrice", columns[3], "expected a non-negative integer");

        data.stageTypes = Split(columns[4], '|');
        for (std::size_t index = 0; index < data.stageTypes.size(); ++index)
        {
            if (data.stageTypes[index].empty())
                return fail("growStages[" + std::to_string(index) + "]", "",
                    "expected a non-empty stage name");
        }

        const auto waterValues = Split(columns[5], '|');
        if (data.stageTypes.size() != waterValues.size())
            return fail("waterStages", columns[5], "expected "
                + std::to_string(data.stageTypes.size()) + " entries to match growStages, found "
                + std::to_string(waterValues.size()));

        for (std::size_t index = 0; index < waterValues.size(); ++index)
        {
            float water = 0.f;
            if (!ParseWater(waterValues[index], water))
                return fail("waterStages[" + std::to_string(index) + "]", waterValues[index],
                    "expected a finite, non-negative number");
            data.waterStages.push_back(water);
        }

        loadedCrops.emplace(cropId, std::move(data));
    }

    if (file.bad())
        return fail("file", "", "failed while reading CSV");
    if (loadedCrops.empty())
        return fail("rows", "0", "expected at least one crop row");

    m_crops.swap(loadedCrops);
    return true;
}

const CropData* CropTable::GetCrop(EItemID id) const
{
    const auto it = m_crops.find(static_cast<int>(id));
    return it != m_crops.end() ? &it->second : nullptr;
}
