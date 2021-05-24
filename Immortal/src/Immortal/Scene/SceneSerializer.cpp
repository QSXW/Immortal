#include "impch.h"
#include "SceneSerializer.h"

#include <fstream>

#include "Entity.h"

#include <yaml-cpp/yaml.h>

namespace YAML {

	template<>
	struct convert<Immortal::Vector::Vector2>
	{
		static Node encode(const Immortal::Vector::Vector2& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			return node;
		}

		static bool decode(const Node& node, Immortal::Vector::Vector2& rhs)
		{
			if (!node.IsSequence() || node.size() != 2)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}
	};

	template<>
	struct convert<Immortal::Vector::Vector3>
	{
		static Node encode(const Immortal::Vector::Vector3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			return node;
		}

		static bool decode(const Node& node, Immortal::Vector::Vector3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};

	template<>
	struct convert<Immortal::Vector::Vector4>
	{
		static Node encode(const Immortal::Vector::Vector4& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			return node;
		}

		static bool decode(const Node& node, Immortal::Vector::Vector4& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};

	template<>
	struct convert<Immortal::Vector::Quaternion>
	{
		static Node encode(const Immortal::Vector::Quaternion& rhs)
		{
			Node node;
			node.push_back(rhs.w);
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			return node;
		}

		static bool decode(const Node& node, Immortal::Vector::Quaternion& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;

			rhs.w = node[0].as<float>();
			rhs.x = node[1].as<float>();
			rhs.y = node[2].as<float>();
			rhs.z = node[3].as<float>();
			return true;
		}
	};
}

namespace Immortal {

	YAML::Emitter& operator<<(YAML::Emitter &out, const Vector::Vector2 &v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter &out, const Vector::Vector3 &v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}


	YAML::Emitter& operator<<(YAML::Emitter &out, const Vector::Vector4 &v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter &out, const Vector::Quaternion &v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.w << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}

	static void SerializeEntity(YAML::Emitter &o, Entity &e)
	{
		o << YAML::BeginMap;
		o << YAML::Key << "Entity" << YAML::Value << "18054007";

		if (e.HasComponent<TagComponent>())
		{
			o << YAML::Key << "TagComponent";
			o << YAML::BeginMap;
			o << YAML::Key << "Tag" << YAML::Value << e.GetComponent<TagComponent>().Tag;
			o << YAML::EndMap;
		}

		if (e.HasComponent<TransformComponent>())
		{
			o << YAML::Key << "TransformComponent";
			o << YAML::BeginMap; // TransformComponent
			
			auto& transform = e.GetComponent<TransformComponent>();

			o << YAML::Key << "Position" << YAML::Value << transform.Position;
			o << YAML::Key << "Rotation" << YAML::Value << transform.Rotation;
			o << YAML::Key << "Scale" << YAML::Value << transform.Scale;

			o << YAML::EndMap; // TransformComponent
		}

		if (e.HasComponent<CameraComponent>())
		{
			o << YAML::Key << "CameraComponent";
			o << YAML::BeginMap; // CameraComponent

			auto& cameraComponent = e.GetComponent<CameraComponent>();
			auto& camera = cameraComponent.Camera;
			o << YAML::Key << "Camera" << YAML::Value;
			o << YAML::BeginMap; // Camera
			o << YAML::Key << "ProjectionType" << YAML::Value << (int)camera.Type();
			o << YAML::Key << "PerspectiveFOV" << YAML::Value << camera.PerspectiveVerticalFOV();
			o << YAML::Key << "PerspectiveNear" << YAML::Value << camera.PerspectiveNearClip();
			o << YAML::Key << "PerspectiveFar" << YAML::Value << camera.PerspectiveFarClip();
			o << YAML::Key << "OrthographicSize" << YAML::Value << camera.OrthographicSize();
			o << YAML::Key << "OrthographicNear" << YAML::Value << camera.OrthographicNearClip();
			o << YAML::Key << "OrthographicFar" << YAML::Value << camera.OrthographicFarClip();
			o << YAML::EndMap; // Camera
			o << YAML::Key << "Primary" << YAML::Value << cameraComponent.Primary;

			o << YAML::EndMap; // CameraComponent
		}

		if (e.HasComponent<SpriteRendererComponent>())
		{
			o << YAML::Key << "SpriteRendererComponent";
			o << YAML::BeginMap; // SpriteRendererComponent

			auto& s = e.GetComponent<SpriteRendererComponent>();
			o << YAML::Key << "Color" << YAML::Value << s.Color;
			if (s.Texture)
				o << YAML::Key << "TextureAssetPath" << YAML::Value << s.Texture->Path();
			o << YAML::Key << "TilingFactor" << YAML::Value << s.TilingFactor;

			o << YAML::EndMap; // SpriteRendererComponent
		}

		o << YAML::EndMap;
	}

	SceneSerializer::SceneSerializer(const Ref<Scene>& scene)
		: mScene(scene)
	{

	}

	void SceneSerializer::Serialize(const std::string & filepath)
	{
		YAML::Emitter o;
		o << YAML::BeginMap;
		o << YAML::Key << "Scene" << YAML::Value << mScene->Name();
		o << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;

		mScene->Registry().each([&](auto id)
			{
				Entity e = { id, mScene.get() };
				if (!e)
				{
					return;
				}
				SerializeEntity(o, e);
			});
		
		o << YAML::EndSeq;
		o << YAML::EndMap;

		std::ofstream fp(filepath);
		if (fp.is_open())
		{
			fp << o.c_str();
		}
	}

	void SceneSerializer::SerializeRuntime(const std::string & filepath)
	{
		IM_CORE_ASSERT(false, "SceneSerializer::SerializeRuntime: Not Implemented");
	}

	bool SceneSerializer::Deserialize(const std::string & filepath)
	{
		YAML::Node data = YAML::LoadFile(filepath);

		IM_CORE_TRACE("{0}: Deserializing Scene '{1}'", __func__, filepath);
		if (!data["Scene"])
		{
			return false;
		}

		auto &entities = data["Entities"];
		if (entities)
		{
			for (auto &e : entities)
			{
				std::string name;
				auto &tagComponent = e["TagComponent"];
				if (tagComponent)
				{
					name = tagComponent["Tag"].as<std::string>();
					IM_CORE_TRACE("Deserialized Object with name = '{0}'", name);
				}

				Entity deserializedEntity = mScene->CreateEntity(name);
		
				auto &transformComponent = e["TransformComponent"];
				if (transformComponent)
				{
					auto &t = deserializedEntity.GetComponent<TransformComponent>();
					t.Position = transformComponent["Position"].as<Vector::Vector3>();
					t.Rotation = transformComponent["Rotation"].as<Vector::Vector3>();
					t.Scale = transformComponent["Scale"].as<Vector::Vector3>();
				}

				auto &cameraComponent = e["CameraComponent"];
				if (cameraComponent)
				{
					auto &c = deserializedEntity.AddComponent<CameraComponent>();
					auto& cameraNode = cameraComponent["Camera"];

					c.Camera = SceneCamera();
					auto& camera = c.Camera;
					if (cameraNode["ProjectionType"])
					{
						camera.SetProjectionType((SceneCamera::ProjectionType)cameraNode["ProjectionType"].as<int>());
					}
					if (cameraNode["PerspectiveFOV"])
					{
						camera.SetPerspectiveVerticalFOV(cameraNode["PerspectiveFOV"].as<float>());
					}
					if (cameraNode["PerspectiveNear"])
					{
						camera.SetPerspectiveNearClip(cameraNode["PerspectiveNear"].as<float>());
					}
					if (cameraNode["PerspectiveFar"])
					{
						camera.SetPerspectiveFarClip(cameraNode["PerspectiveFar"].as<float>());
					}
					if (cameraNode["OrthographicSize"])
					{
						camera.SetOrthographicSize(cameraNode["OrthographicSize"].as<float>());
					}
					if (cameraNode["OrthographicNear"])
					{
						camera.SetOrthographicNearClip(cameraNode["OrthographicNear"].as<float>());
					}
					if (cameraNode["OrthographicFar"])
					{
						camera.SetOrthographicFarClip(cameraNode["OrthographicFar"].as<float>());
					}
					c.Primary = cameraComponent["Primary"].as<bool>();
				}

				auto spriteRendererComponent = e["SpriteRendererComponent"];
				if (spriteRendererComponent)
				{
					auto& component = deserializedEntity.AddComponent<SpriteRendererComponent>();
					component.Color = spriteRendererComponent["Color"].as<Vector::Vector4>();
					component.TilingFactor = spriteRendererComponent["TilingFactor"].as<float>();
				}
			}
			return true;
		}

		return false;
	}

	bool SceneSerializer::DeserializeRuntime(const std::string & filepath)
	{
		IM_CORE_ASSERT(false, "SceneSerializer::DeserializeRuntime: Not Implemented");
		return false;
	}

}