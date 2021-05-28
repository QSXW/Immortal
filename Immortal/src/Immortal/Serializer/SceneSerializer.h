#pragma once

#include "ImmortalCore.h"

#include "Immortal/Scene/Scene.h"

namespace Immortal {

	class IMMORTAL_API SceneSerializer
	{
	public:
		SceneSerializer(const Ref<Scene> &scene);
		~SceneSerializer() = default;
		
		void Serialize(const std::string &filepath);
		void SerializeRuntime(const std::string &filepath);

		bool Deserialize(const std::string &filepath);
		bool DeserializeRuntime(const std::string& filepath);

	private:
		Ref<Scene> mScene;
	};

}

