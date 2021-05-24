#pragma once

#include "ImmortalCore.h"
#include "Immortal/Events/Event.h"

namespace Immortal
{
	class IMMORTAL_API Layer
	{
	public:
		Layer(const std::string &name = "Layer");
		virtual ~Layer();

		virtual void OnAttach() { }
		virtual void OnDetach() { }
		virtual void OnUpdate() { }
		virtual void OnGuiRender() { }
		virtual void OnEvent(Event &e) { }
		virtual void Begin() { }
		virtual void End() { }

		inline const std::string &Name() const { return mDebugName; }

	protected:
		std::string mDebugName;
	};
}

