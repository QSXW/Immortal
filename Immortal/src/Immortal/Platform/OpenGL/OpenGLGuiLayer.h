#pragma once
#include "Immortal/ImGui/GuiLayer.h"

namespace Immortal
{
	class OpenGLGuiLayer : virtual public GuiLayer
	{
	public:
		using Super = GuiLayer;

	public:
		OpenGLGuiLayer();
		~OpenGLGuiLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnEvent(Event &e) override;
		virtual void OnGuiRender() override;

		virtual void Begin() override;
		virtual void End() override;
	};

}

