#pragma once

#include "Entity.h"
#include "Immortal/Interface/Delegate.h"

namespace Immortal {

	class __declspec(dllexport) ScriptableObject
	{
	public:
		virtual ~ScriptableObject() { }

		template <class T>
		T &GetComponent()
		{
			return mEntity.GetComponent<T>();
		}

		template <class T, class... Args>
		T &AddComponent(Args&&... args)
		{
			return mEntity.AddComponent<T>(std::forward<Args>(args)...);
		}

	protected:
		virtual void OnStart() { }
		virtual void OnDestroy() { }
		virtual void OnUpdate() { }

	public:
		Entity mEntity;
		friend class Scene;
		friend struct NativeScriptComponent;
	};

	using GameObject = ScriptableObject;

	struct NativeScriptComponent : public Component
	{
		enum class Status : int
		{
			Ready,
			NotLoaded
		};
		std::string Module;
		Ref<Delegate<void()>> delegate;
		NativeScriptComponent::Status Status;

		void Bind(Entity e, ScriptableObject *script)
		{
			script->mEntity = e;
			script->OnStart();
			delegate->Bind<ScriptableObject, &ScriptableObject::OnUpdate>(script);
		}

		NativeScriptComponent() : Component(Component::Script), Status(NativeScriptComponent::Status::NotLoaded)
		{
			delegate = CreateRef<Delegate<void()>>();
		}

		NativeScriptComponent(const std::string &module)
			: Component(Component::Script), Module(module), Status(NativeScriptComponent::Status::NotLoaded)
		{
			delegate = CreateRef<Delegate<void()>>();
		}

		__forceinline void OnRuntime()
		{
			delegate->Invoke();
		}

		~NativeScriptComponent()
		{

		}
	};
}
