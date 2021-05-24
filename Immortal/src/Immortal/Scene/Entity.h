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
			: mHandle(handle), mScene(scene)
		{

		}

		Entity(int handle, Scene *scene)
			: mHandle(entt::entity(handle)), mScene(scene)
		{

		}

		~Entity() { }

		template <class T>
		void RemoveComponent()
		{
			mScene->mRegistry.remove<T>(mHandle);
		}

		template <class T, class... Args>
		T &AddComponent(Args&&... args)
		{
			if (HasComponent<T>())
			{
				IM_CORE_WARN("Add component to a enity which alread has an identical component. The previous one would be remove.");
				RemoveComponent<T>();
			}
			return mScene->mRegistry.emplace<T>(mHandle, std::forward<Args>(args)...);
		}

		template <class T>
		T &GetComponent() const
		{
			return mScene->mRegistry.get<T>(mHandle);
		}

		template <class T>
		bool HasComponent()
		{
			return mScene->mRegistry.has<T>(mHandle);
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
			return (uint64_t)mHandle;
		}

		operator entt::entity() const
		{
			return mHandle;
		}

		operator bool() const
		{
			return uint64_t(mHandle) && mScene;
		}

		bool operator==(const Entity &other) const
		{
			return mHandle == other.mHandle && mScene == other.mScene;
		}

		bool operator!=(const Entity &other) const
		{
			return !(*this == other);
		}

	private:
		entt::entity mHandle{ entt::null };
		Scene *mScene{ nullptr };

		friend class Scene;
	};

}

