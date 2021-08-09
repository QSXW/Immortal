#pragma once

#include "ImmortalCore.h"

#include "entt.hpp"
#include "Scene.h"
#include "Component.h"

namespace Immortal {

	class IMMORTAL_API Entity
	{
	public:
		Entity() = default;
		Entity(entt::entity handle, Scene *scene)
			: handle(handle), mScene(scene)
		{

		}

		Entity(int handle, Scene *scene)
			: handle(entt::entity(handle)), mScene(scene)
		{

		}

		~Entity() { }

		template <class T>
		void RemoveComponent()
		{
			mScene->mRegistry.remove<T>(handle);
		}

		template <class T, class... Args>
		T &AddComponent(Args&&... args)
		{
			if (HasComponent<T>())
			{
				IM_CORE_WARN("Add component to a enity which alread has an identical component. The previous one would be remove.");
				RemoveComponent<T>();
			}
			return mScene->mRegistry.emplace<T>(handle, std::forward<Args>(args)...);
		}

		template <class T>
		T &GetComponent() const
		{
			return mScene->mRegistry.get<T>(handle);
		}

		template <class T>
		bool HasComponent()
		{
			return mScene->mRegistry.has<T>(handle);
		}

		TransformComponent &Transform() 
		{
			return GetComponent<TransformComponent>();
		}

		const Vector::mat4 &Transform() const
		{
			return GetComponent<TransformComponent>().Transform();
		}

		operator uint64_t() const
		{
			return (uint64_t)handle;
		}

		operator entt::entity() const
		{
			return handle;
		}

		operator bool() const
		{
			return uint64_t(handle) && mScene;
		}

		bool operator==(const Entity &other) const
		{
			return handle == other.handle && mScene == other.mScene;
		}

		bool operator!=(const Entity &other) const
		{
			return !(*this == other);
		}

	private:
		entt::entity handle{ entt::null };
		Scene *mScene{ nullptr };

		friend class Scene;
	};

}

