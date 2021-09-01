#pragma once
#include <Immortal.h>
#include "Framework/Main.h"
#include "Platform/Vulkan/vkcommon.h"

using namespace Immortal;

class VulkanLayer : public Layer
{
	VulkanLayer();
	virtual ~VulkanLayer() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnUpdate() override;
};

class VulkanSample : public Application
{
public:
	VulkanSample() : Application({ UNICODE8("Vulkan Test"), 1920, 1080 })
	{
		auto renderContext = this->GetWindow().Context();

		VkSemaphoreCreateInfo createInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };

		// Vulkan::Check(vkCreateSemaphore());
	}

	~VulkanSample()
	{

	}
};

Immortal::Application* Immortal::CreateApplication()
{
	RendererAPI::SetAPI(RendererAPI::Type::VulKan);
	return new VulkanSample();
}
