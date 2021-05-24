#include "impch.h"
#include "GuiLayer.h"

#include <imgui.h>
#include "ImGuizmo.h"

#include "Immortal/Core/Application.h"
#include "Immortal/Render/Renderer.h"

#include "Immortal/Platform/OpenGL/OpenGLGuiLayer.h"
#include "Immortal/Platform/Vulkan/VulkanGuiLayer.h"

namespace Immortal
{
	GuiLayer* GuiLayer::Create() NOEXCEPT
	{
		switch (Renderer::API())
		{
			case RendererAPI::Type::OpenGL:
				return dynamic_cast<GuiLayer*>(new OpenGLGuiLayer());
			case RendererAPI::Type::VulKan:
				return dynamic_cast<GuiLayer*>(new VulkanGuiLayer());
			default:
				IM_CORE_ASSERT(false, "RendererAPI::Target API is currently not supported! But it should be On schedule.");
				return nullptr;
		}
	}

	GuiLayer::GuiLayer()
		: Layer("GuiLayer")
	{

	}

	GuiLayer::~GuiLayer()
	{

	}

	void GuiLayer::OnAttach()
	{	
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::StyleColorsDark();

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
		io.ConfigFlags  |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		io.Fonts->AddFontFromFileTTF(
			"assets/Fonts/NotoSansCJKsc-Regular.otf",
			18.0f,
			NULL,
			io.Fonts->GetGlyphRangesChineseFull()
		);

		io.FontDefault = io.Fonts->AddFontFromFileTTF(
			"assets/Fonts/NotoSansCJKsc-DemiLight.otf",
			18.0f,
			NULL,
			io.Fonts->GetGlyphRangesChineseFull()
		);
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		this->SetThemeColors();
	}	
		
	void GuiLayer::OnDetach()
	{
		ImGui::DestroyContext();
		ImGuizmo::BeginFrame();
	}

	void GuiLayer::Begin()
	{
		ImGui::NewFrame();
	}

	void GuiLayer::End()
	{
		ImGui::Render();
	}

	void GuiLayer::SetThemeColors()
	{
		auto& colors = ImGui::GetStyle().Colors;
		
		// const ImVec4 thereColor = { 0.149, 0.149, 0.149, 1.0f };
		const ImVec4 thereColor = { 0.18f, 0.18f, 0.18f, 1.0f };
		colors[ImGuiCol_WindowBg] = thereColor;

		//// Headers
		colors[ImGuiCol_Header] = ImVec4{ (float)(93.0 / 255.0), (float)(111.0 / 255), (float)(125.0 / 255.0), 1.0f };
		colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_HeaderActive] = ImVec4{ (float)(93.0 / 255.0), (float)(111.0 / 255), (float)(125.0 / 255.0), 1.0f };

		//// Buttons
		colors[ImGuiCol_Button] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_ButtonActive] = ImVec4{ 0.3f, 0.3f, 0.3f, 1.0f };

		// Frame BG
		colors[ImGuiCol_FrameBg] = { 0.25f, 0.25f, 0.25f, 1.0f };
		colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		//// Tabs
		colors[ImGuiCol_Tab] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TabHovered] = ImVec4{ 0.38f, 0.3805f, 0.381f, 1.0f };
		colors[ImGuiCol_TabActive] = ImVec4{ 0.28f, 0.2805f, 0.281f, 1.0f };
		colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };

		//// Title
		colors[ImGuiCol_TitleBg] = thereColor;
		colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.3f, 0.3f, 0.3f, 1.0f };
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		//// Checkbox
		colors[ImGuiCol_CheckMark] = ImVec4{ 0.91f, 0.0196f, 0.15294118f, 1.0f };

		// Slider
		colors[ImGuiCol_SliderGrab] = ImVec4{ 0.3f, 0.3f, 0.3f, 1.0f };
		colors[ImGuiCol_SliderGrabActive] = ImVec4{ 1.0f, 1.0f, 1.0f, 1.0f };

		// Docking
		colors[ImGuiCol_DockingPreview] = ImVec4{ 1.0f, 1.0f, 1.0f, 1.0f };
	}

	void GuiLayer::OnEvent(Event &e)
	{
		if (mBlockEvents)
		{
			ImGuiIO &io = ImGui::GetIO();
			e.Handled |= e.IsInCategory(EventCategoryMouse) & io.WantCaptureMouse;
			e.Handled |= e.IsInCategory(EventCategoryKeyboard) & io.WantCaptureKeyboard;
		}
	}

	void GuiLayer::OnGuiRender()
	{

	}

}