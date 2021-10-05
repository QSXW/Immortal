#pragma once

#include "ImmortalCore.h"

#include "entt.hpp"

#include "Editor/EditorCamera.h"
#include "ObserverCamera.h"

#include "Render/Texture.h"
#include "Render/Environment.h"
#include "Render/Mesh.h"
#include "Render/Framebuffer.h"

namespace Immortal {

	struct Light
	{
		Vector::Vector3 Direction{ 0.0f, 0.0f, 0.0f };
		Vector::Vector3 Radiance{ 0.0f, 0.0f, 0.0f };

		bool Enabled = false;
	};

	struct PointLight
	{
		Vector::Vector3 Position{ 0.0f, 0.0f, 0.0f };
		Vector::Vector3 Radiance{ 0.0f, 0.0f, 0.0f };
	};

	struct DirectionalLight
	{
		Vector::Vector3 Direction{ 0.0f, 0.0f, 0.0f };
		Vector::Vector3 Radiance{ 0.0f, 0.0f, 0.0f };
	};

	// This type of light needs the most time to calculate
	struct SpotLight
	{
		Vector::Vector3 Direction{ 0.0f, 0.0f, 0.0f };
		float Falloff{ 1.0 };
		float Theta; // the radian angle of the spotlight's inner cone
		float Phi;   // the angle for the outer cone of light.
	};

	struct LightEnvironment
	{
		static constexpr int LightNumbers = 3;
		float pitch = 0.0f;
		float yaw   = 0.0f;
		Light lights[LightNumbers];
	};

	class Entity;
	class IMMORTAL_API Scene
	{
	public:
		Scene(const std::string &debugName="Untitled", bool isEditorScene = false);
		~Scene();

		void OnUpdate();
		void OnEvent();

		void OnRenderRuntime();
		void OnRenderEditor(const EditorCamera &editorCamera);

		auto& Registry() { return mRegistry; }

		Entity CreateEntity(const std::string &name = "");
		void DestroyEntity(Entity &e);

		void SetViewportSize(const Vector::Vector2 &size);

		Entity PrimaryCameraEntity();

	public:
		const char *Name() const { return mDebugName.c_str(); }

	public:
		using EntityMap = std::unordered_map<uint64_t, Entity>;

	private:
		std::string mDebugName;
		entt::entity mEntity;
		entt::registry mRegistry; // Entity Context: a container that contains our enties

		EntityMap mEntityMap;
		std::vector<Entity *> mMeshEntities;

		Vector::Vector2 mViewportSize{ 0.0f, 0.0f };

		std::shared_ptr<TextureCube> mSkyboxTexture;

		std::shared_ptr<Environment> mEnvironment;
		std::shared_ptr<Mesh> mSkyBox;
		LightEnvironment mLightEnvironment;

		std::shared_ptr<UniformBuffer> mTransformUniformBuffer;
		std::shared_ptr<UniformBuffer> mShadingUniformBuffer;

		std::shared_ptr<VertexArray> mVertexArray;

		std::shared_ptr<VertexArray> mToneMap;
		std::shared_ptr<Framebuffer> mFramebuffer;

		friend class Entity;
		friend class SceneHierarchyPanel;
		friend class EditorLayer;

	/* Camera Related */
	private:
		ObserverCamera mObserverCamera;
	public:
		const ObserverCamera &Observer() const NOEXCEPT { return mObserverCamera; }
	};


}
