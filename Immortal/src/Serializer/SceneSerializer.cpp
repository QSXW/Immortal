#include "impch.h"
#include "SceneSerializer.h"

#include <fstream>
#include "Scene/Entity.h"

#include "json.h"

using json = nlohmann::json;

namespace Immortal {

	void get_to(Vector3 &v)
	{

	}

	void to_json(json &j, const TransformComponent &t)
	{

	}

	void from_json(const json& j, TransformComponent &p)
	{

	}

	SceneSerializer::SceneSerializer(const Ref<Scene> &scene)
		: mScene(scene)
	{

	}

	void SceneSerializer::Serialize(const std::string & filepath)
	{
		
	}

	void SceneSerializer::SerializeRuntime(const std::string & filepath)
	{
		SLASSERT(false && "SceneSerializer::SerializeRuntime: Not Implemented");
	}

	bool SceneSerializer::Deserialize(const std::string & filepath)
	{
		return false;
	}

	bool SceneSerializer::DeserializeRuntime(const std::string & filepath)
	{
		SLASSERT(false && "SceneSerializer::DeserializeRuntime: Not Implemented");
		return false;
	}

}