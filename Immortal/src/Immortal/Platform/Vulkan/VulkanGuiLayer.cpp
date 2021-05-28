#include "impch.h"
#include "VulkanGuiLayer.h"

#include "Immortal/Core/Application.h"
#include "backends/imgui_impl_vulkan.h"

namespace Immortal
{
	VulkanGuiLayer::VulkanGuiLayer()
	{

	}

	VulkanGuiLayer::~VulkanGuiLayer()
	{

	}

	void VulkanGuiLayer::OnAttach()
	{
		Super::OnAttach();
		Application *app = Application::App();

		std::vector<VkDescriptorPoolSize> poolSizes = { 
			VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER , 1 }
		};

	}

	void VulkanGuiLayer::OnDetach()
	{
		Super::OnDetach();
	}

	void VulkanGuiLayer::OnEvent(Event & e)
	{

	}

	void VulkanGuiLayer::OnGuiRender()
	{

	}

	void VulkanGuiLayer::Begin()
	{
		Super::Begin();
	}

	void VulkanGuiLayer::End()
	{
		Super::End();

		ImGuiIO& io = ImGui::GetIO();
		Application *app = Application::App();
		io.DisplaySize = ImVec2((float)app->Width(), (float)app->Height());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}
}