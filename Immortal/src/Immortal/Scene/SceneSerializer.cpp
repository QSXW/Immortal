#include "impch.h"
#include "SceneSerializer.h"

#include <fstream>

#include "Entity.h"


namespace Immortal {

	SceneSerializer::SceneSerializer(const Ref<Scene>& scene)
		: mScene(scene)
	{

	}

	void SceneSerializer::Serialize(const std::string & filepath)
	{
		
	}

	void SceneSerializer::SerializeRuntime(const std::string & filepath)
	{
		IM_CORE_ASSERT(false, "SceneSerializer::SerializeRuntime: Not Implemented");
	}

	bool SceneSerializer::Deserialize(const std::string & filepath)
	{
		return false;
	}

	bool SceneSerializer::DeserializeRuntime(const std::string & filepath)
	{
		IM_CORE_ASSERT(false, "SceneSerializer::DeserializeRuntime: Not Implemented");
		return false;
	}

}