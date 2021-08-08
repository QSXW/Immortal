#pragma once

#include "Framework/Layer.h"

#include "Event/KeyEvent.h"
#include "Event/MouseEvent.h"

namespace Immortal
{
	class IMMORTAL_API GuiLayer : public Layer
	{
	public:
		static GuiLayer* GuiLayer::Create() NOEXCEPT;

	public:
		GuiLayer();
		~GuiLayer();

		void OnUpdate() { }
		void OnAttach() override;
		void OnDetach() override;
		void OnEvent(Event &e) override;
		void OnGuiRender() override;

		void Begin() override;
		void End() override;

		void BlockEvent(bool block) { mBlockEvents = block;  }
		void SetThemeColors();

	private:
		bool mBlockEvents = true;
		float mTime = 0.0f;
	};

}