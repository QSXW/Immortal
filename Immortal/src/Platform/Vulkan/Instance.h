#pragma once

#include "ImmortalCore.h"

#include "vkcommon.h"

namespace Immortal
{
namespace Vulkan {
	class PhysicalDevice;
	class Instance
	{
	public:
		Instance() = default;

		Instance(const char*                                  applicationName,
			     const std::unordered_map<const char*, bool> &requiredExtension        = {},
			     const std::vector<const char*>&              requiredValidationLayers = {},
			     bool                                         headless                 = false,
			     UINT32                                       apiVersion               = VK_API_VERSION_1_0);

		Instance(VkInstance instance);

		~Instance();

		void QueryGraphicsProcessingUnits() NOEXCEPT;

		PhysicalDevice& SuitableGraphicsProcessingUnit() NOEXCEPT;

		bool IsEnabled(const char* extension) const NOEXCEPT;

		bool CheckValidationLayerSupport();
	public:
		DefineGetHandleFunc(VkInstance)

	private:
		VkInstance handle{ VK_NULL_HANDLE };
		std::vector<const char*> mEnabledExtensions{};

#if defined ( _DEBUG ) || defined ( VKB_VALIDATION_LAYERS )
		VkDebugUtilsMessengerEXT mDebugUtilsMessengers{ VK_NULL_HANDLE };
		VkDebugReportCallbackEXT mDebugReportCallback{ VK_NULL_HANDLE };
#endif

		std::vector<Unique<PhysicalDevice>> mGraphicsProcessingUnits{};
	};
}
}

