#include "PCH.h"
#include "ObjectLoader.h"
#include "ObjectFactory.h"
#include "DataTable.h"
#include "GameObject.h"

bool ObjectLoader::LoadJson(const std::string& path, rapidjson::Document& doc)
{
	std::string json = LoadFile(path);
	if (json.empty()) return false;

	doc.Parse(json.c_str());
	if (!doc.IsObject() || !doc.HasMember("objects"))
		return false;

	return true;
}

glm::vec3 ObjectLoader::ReadVec3(const rapidjson::Value& v)
{
	return glm::vec3(v[0].GetFloat(), v[1].GetFloat(), v[2].GetFloat());
}

void ObjectLoader::FlipCoord(glm::vec3& pos, glm::vec3& rot)
{
	pos.x = -pos.x;
	rot.y = -rot.y;
}

std::vector<std::shared_ptr<GameObject>> ObjectLoader::LoadStaticObjects(const std::string& path)
{
	std::vector<std::shared_ptr<GameObject>> results;

	rapidjson::Document doc;
	if (!LoadJson(path, doc)) {
		LOG_E("JSON load fail: %s", path.c_str());
		return results;
	}

	auto arr = doc["objects"].GetArray();
	for (auto& obj : arr)
	{
		std::string type = obj["type"].GetString();
		glm::vec3 pos = ReadVec3(obj["pos"]);
		glm::vec3 rot = ReadVec3(obj["rot"]);
		glm::vec3 scale(1.0f);

		FlipCoord(pos, rot);

		auto info = DataTable::GetObjectInfo(type);
		if (!info) continue;

		auto inst = ObjectFactory::Create(type, pos, rot, scale, info);
		inst->m_name = type;
		inst->Init();

		results.push_back(inst);
	}

	return results;
}

std::vector<std::shared_ptr<GameObject>> ObjectLoader::LoadInteractableObjects(const std::string& path)
{
	std::vector<std::shared_ptr<GameObject>> results;

	rapidjson::Document doc;
	if (!LoadJson(path, doc)) {
		LOG_E("JSON load fail: %s", path.c_str());
		return results;
	}

	auto arr = doc["objects"].GetArray();
	for (auto& obj : arr)
	{
		std::string type = obj["type"].GetString();
		glm::vec3 pos = ReadVec3(obj["pos"]);
		glm::vec3 rot = ReadVec3(obj["rot"]);
		glm::vec3 scale = ReadVec3(obj["scale"]);

		FlipCoord(pos, rot);

		auto info = DataTable::GetObjectInfo((type == "Field") ? "Thick_Ground" : type);
		if (!info) continue;

		auto inst = ObjectFactory::Create(type, pos, rot, scale, info);
		inst->m_name = type;
		inst->Init();

		results.push_back(inst);
	}

	return results;
}

std::vector<std::shared_ptr<GameObject>> ObjectLoader::LoadEndingObjects()
{
	std::vector<std::shared_ptr<GameObject>> results;

	const int N = 50;
	const float SPACE = 2.0f;

	for (int x = 0; x < N/2; x++)
	{
		for (int z = 0; z < N; z++)
		{
			std::string type = "Water";

			glm::vec3 pos(x * SPACE, 0.0f, z * SPACE);
			glm::vec3 rot(0.f);
			glm::vec3 scale(1.f);

			FlipCoord(pos, rot);

			const ObjectInfo* info = DataTable::GetObjectInfo(type);
			if (!info) continue;

			auto inst = ObjectFactory::Create(type, pos, rot, scale, info);
			inst->m_name = type;
			inst->Init();

			results.push_back(inst);
		}
	}

	glm::vec3 boatPos(0.f, 0.f, 50.f);
	glm::vec3 boatRot(0.f, 135.f, 0.f);
	glm::vec3 boatScale(1.5f);

	if (auto info = DataTable::GetObjectInfo("Boat"))
	{
		auto boat = ObjectFactory::Create("Boat", boatPos, boatRot, boatScale, info);
		boat->m_name = "Boat";
		boat->Init();
		results.push_back(boat);
	}

	return results;
}
