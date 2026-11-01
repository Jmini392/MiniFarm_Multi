#pragma once
#include <rapidjson/document.h>

class GameObject;
class ObjectLoader
{
public:
	static std::vector<std::shared_ptr<GameObject>> LoadStaticObjects(const std::string& path);
	static std::vector<std::shared_ptr<GameObject>> LoadInteractableObjects(const std::string& path);
	static std::vector<std::shared_ptr<GameObject>> LoadEndingObjects();
private:
	static  bool LoadJson(const std::string& path, rapidjson::Document& doc);

	static glm::vec3 ReadVec3(const rapidjson::Value& v);
	static void FlipCoord(glm::vec3& pos, glm::vec3& rot);
};