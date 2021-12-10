#include "impch.h"
#include "SceneSerializer.h"

#include <fstream>
#include "Scene/Entity.h"

namespace Immortal
{


SceneSerializer::SceneSerializer(const std::shared_ptr<Scene> &scene)
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