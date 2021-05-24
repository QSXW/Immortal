#pragma once

#include "ImmortalCore.h"

#include "VulkanCommon.h"

namespace Immortal
{
	class VulkanPhysicalDevice;
	class VulkanInstance
	{
	public:
		VulkanInstance() = default;

		VulkanInstance(const char                                   *ApplicationName,
					   const std::unordered_map<const char *, bool> &requiredExtension        = {},
					   const std::vector<const char *>              &requiredValidationLayers = {},
					   bool                                          headless                 = false,
					   UINT32                                        apiVersion               = VK_API_VERSION_1_0);

		VulkanInstance(VkInstance instance);

		~VulkanInstance();

		DefineGetHandleFunc(VkInstance)

		void QueryGraphicsProcessingUnits() NOEXCEPT;

		VulkanPhysicalDevice &SuitableGraphicsProcessingUnit() NOEXCEPT;

#if defined( IM_DEBUG )
		bool CheckValidationLayerSupport() NOEXCEPT;
#endif

	private:
		VkInstance mHandle{ VK_NULL_HANDLE };
		std::vector<const char *> mEnabledExtensions{};

#if defined ( IM_DEBUG ) || defined ( VKB_VALIDATION_LAYERS )
		VkDebugUtilsMessengerEXT mDebugUtilsMessengers{ VK_NULL_HANDLE };
		VkDebugReportCallbackEXT mDebugReportCallback{ VK_NULL_HANDLE };
#endif

		std::vector<Unique<VulkanPhysicalDevice>> mGraphicsProcessingUnits{};
	};
}

