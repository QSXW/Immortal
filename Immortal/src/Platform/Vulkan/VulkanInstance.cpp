#include "impch.h"
#include "VulkanInstance.h"

#include "VulkanDevice.h"

namespace Immortal
{
	namespace Vulkan
	{
#if		IMMORTAL_CHECK_DEBUG
		static VKAPI_ATTR VkBool32 VKAPI_CALL DebugUtilsMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT *callbackData,
			void *                                      userData)
		{
			// Log debug messge
			if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
			{
				IM_CORE_WARN("{0} - {1}: {2}", callbackData->messageIdNumber, callbackData->pMessageIdName, callbackData->pMessage);
			}
			else if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
			{
				IM_CORE_ERROR("{0} - {1}: {2}", callbackData->messageIdNumber, callbackData->pMessageIdName, callbackData->pMessage);
			}

			return VK_FALSE;
		}

		static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT /*type*/,
			uint64_t /*object*/, size_t /*location*/, int32_t /*message_code*/,
			const char *LayerPrefix, const char *message, void * /*userData*/)
		{
			if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
			{
				IM_CORE_ERROR("{0}: {1}", LayerPrefix, message);
			}
			else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
			{
				IM_CORE_WARN("{0}: {1}", LayerPrefix, message);
			}
			else if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
			{
				IM_CORE_WARN("{0}: {1}", LayerPrefix, message);
			}
			else
			{
				IM_CORE_INFO("{0}: {1}", LayerPrefix, message);
			}
			return VK_FALSE;
		}
#endif

		static bool ValidateLayers(const std::vector<const char *> &required, const std::vector<VkLayerProperties> &available)
		{
			for (auto &layer : required)
			{
				bool found = false;
				for (auto &availableLayer : available)
				{
					if (Equals(layer, availableLayer.layerName))
					{
						found = true;
						break;
					}
				}
				if (!found)
				{
					IM_CORE_ERROR("Validation Layer {} not found", layer);
					return false;
				}
			}
			
			return true;
		}
	}

#define VkEnableExtension(msg) IM_CORE_INFO("{0} is available. Enabling it!", msg);

	/**
	 * @breif Create a vulkan instance object.
	 * @param headless is an instance extension. It provides a mechanism to create VkSurfaceKHR objects independently of any window system or display device. 
	 * @param requiredValidationLayers is the layers that you wanna use.
	 */
	VulkanInstance::VulkanInstance(const char                                   *applicationName,
		                           const std::unordered_map<const char *, bool> &requiredExtension,
		                           const std::vector<const char *>              &requiredValidationLayers,
		                           bool                                          headless,
		                           UINT32                                        apiVersion)
	{
		UINT32 instanceExtensionCount;
		Vulkan::Check(vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, nullptr));

		std::vector<VkExtensionProperties> availableInstanceExtensions(instanceExtensionCount);
		Vulkan::Check(vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, availableInstanceExtensions.data()));

#if     IMMORTAL_CHECK_DEBUG
		bool debugUtils = false;
#endif
		bool headlessExtension = false;
		for (auto &ext : availableInstanceExtensions)
		{
#if			IMMORTAL_CHECK_DEBUG
			if (Vulkan::Equals(ext.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
			{

				debugUtils = true;
				VkEnableExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
				mEnabledExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			}
#endif		
			if (headless && Vulkan::Equals(ext.extensionName, VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME))
			{
				headlessExtension = true;
				VkEnableExtension(VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME);
				mEnabledExtensions.emplace_back(VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME);
			}

			/**
			 * @brief VK_KHR_get_physical_device_properties2 is a prerequisite of VK_KHR_performance_query
			   which will be used for stats gathering where available.
			*/
			if (Vulkan::Equals(ext.extensionName, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME))
			{
				VkEnableExtension(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
			}
		}
		if (headless && !headlessExtension)
		{
			IM_CORE_WARN("{0} is not available. Disabling swapchain creation", VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME);
		}
		else
		{
			mEnabledExtensions.emplace_back(VK_KHR_SURFACE_EXTENSION_NAME);
		}

#if     IMMORTAL_CHECK_DEBUG
		if (!debugUtils)
		{
			mEnabledExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		}
#endif

		for (auto &ext : requiredExtension)
		{
			auto extensionName = ext.first;
			auto isOptional    = ext.second;
			if (std::find_if(availableInstanceExtensions.begin(), availableInstanceExtensions.end(), [&extensionName](VkExtensionProperties &availableExtension)
				{
					return Vulkan::Equals(availableExtension.extensionName, extensionName);
				}) == availableInstanceExtensions.end())
			{
				if (isOptional)
				{
					IM_CORE_WARN("Optional instance extension {0} not available, some features may be disabled", extensionName);
				}
				else
				{
					IM_CORE_ERROR("Required instance extension {0} not available, cannot run", extensionName);
					IM_CORE_ERROR("Required instance extensions are missing.");
				}
			}
			else
			{
				mEnabledExtensions.emplace_back(extensionName);
			}
		}

		UINT32 instanceLayerCount;
		Vulkan::Check(vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr));

		std::vector<VkLayerProperties> supportedValidationLayers(instanceLayerCount);
		Vulkan::Check(vkEnumerateInstanceLayerProperties(&instanceLayerCount, supportedValidationLayers.data()));

		if (Vulkan::ValidateLayers(requiredValidationLayers, supportedValidationLayers))
		{
			IM_CORE_INFO("Enabled Validation Layers: ");
			for (const auto &layer : requiredValidationLayers)
			{
				IM_CORE_INFO("  \t{0}", layer);;
			}
		}
		else
		{
			IM_CORE_ERROR("Required validation layers are missing.");
		}

		// @optional
		VkApplicationInfo appInfo{};
		appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pNext              = nullptr;
		appInfo.pApplicationName   = applicationName;
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName        = applicationName;
		appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion         = apiVersion;

		// @required
		VkInstanceCreateInfo instanceInfo{};
		instanceInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceInfo.pNext                   = nullptr;
		instanceInfo.pApplicationInfo        = &appInfo;
		instanceInfo.enabledExtensionCount   = static_cast<UINT32>(mEnabledExtensions.size());
		instanceInfo.ppEnabledExtensionNames = mEnabledExtensions.data();
		instanceInfo.enabledLayerCount       = static_cast<UINT32>(requiredValidationLayers.size());
		instanceInfo.ppEnabledLayerNames     = requiredValidationLayers.data();

#if     IMMORTAL_CHECK_DEBUG
		VkDebugUtilsMessengerCreateInfoEXT debugUtilsCreateInfo = { VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
		VkDebugReportCallbackCreateInfoEXT debugReportCreateInfo = { VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT };

		if (debugUtils)
		{
			debugUtilsCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
			debugUtilsCreateInfo.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			debugUtilsCreateInfo.pfnUserCallback = Vulkan::DebugUtilsMessengerCallback;

			instanceInfo.pNext = &debugUtilsCreateInfo;
		}
		else
		{
			debugReportCreateInfo.flags       = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
			debugReportCreateInfo.pfnCallback = Vulkan::DebugCallback;

			instanceInfo.pNext = &mDebugReportCallback;
		}
#endif

		Vulkan::Check(vkCreateInstance(&instanceInfo, nullptr, &mHandle));

#if     IMMORTAL_CHECK_DEBUG
		if (debugUtils)
		{
			auto vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(mHandle, "vkCreateDebugUtilsMessengerEXT");
			IM_CORE_ASSERT(vkCreateDebugUtilsMessengerEXT != nullptr, "");
			Vulkan::Check(vkCreateDebugUtilsMessengerEXT(mHandle, &debugUtilsCreateInfo, nullptr, &mDebugUtilsMessengers));
		}
		else
		{
			auto vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(mHandle, "vkCreateDebugReportCallbackEXT");
			IM_CORE_ASSERT(vkCreateDebugReportCallbackEXT != nullptr, "");
			Vulkan::Check(vkCreateDebugReportCallbackEXT(mHandle, &debugReportCreateInfo, nullptr, &mDebugReportCallback));
		}
#endif

		QueryGraphicsProcessingUnits();
	}

	VulkanInstance::VulkanInstance(VkInstance instance)
		: mHandle(instance)
	{
		IM_CORE_ASSERT(mHandle != VK_NULL_HANDLE, "Instance not valid");
		QueryGraphicsProcessingUnits();
	}

	VulkanInstance::~VulkanInstance()
	{
#if defined( IM_DEBUG )
		if (mDebugUtilsMessengers)
		{
			auto vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(mHandle, "vkDestroyDebugUtilsMessengerEXT");
			vkDestroyDebugUtilsMessengerEXT(mHandle, mDebugUtilsMessengers, nullptr);
		}
		if (mDebugReportCallback)
		{
			auto vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(mHandle, "vkDestroyDebugReportCallbackEXT");
			vkDestroyDebugReportCallbackEXT(mHandle, mDebugReportCallback, nullptr);
		}
#endif
		if (mHandle)
		{
			vkDestroyInstance(mHandle, nullptr);
		}
	}

	/**
	 * @breif Look for and select a graphics card in the system that supports the features we need.
	 * 
	 */
	void VulkanInstance::QueryGraphicsProcessingUnits() NOEXCEPT
	{
		UINT32 physicalDeviceCount{ 0 };
		Vulkan::Check(vkEnumeratePhysicalDevices(mHandle, &physicalDeviceCount, nullptr));

		IM_CORE_ASSERT(physicalDeviceCount >= 1, "Couldn't find a physical device that supports Vulkan.");

		std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
		Vulkan::Check(vkEnumeratePhysicalDevices(mHandle, &physicalDeviceCount, physicalDevices.data()));

		for (auto &d : physicalDevices)
		{
			mGraphicsProcessingUnits.emplace_back(MakeUnique<VulkanPhysicalDevice>(*this, d));
		}
	}

	VulkanPhysicalDevice &VulkanInstance::SuitableGraphicsProcessingUnit() NOEXCEPT
	{
		IM_CORE_ASSERT(!mGraphicsProcessingUnits.empty(), LOGB("该主机没有任何可用的显卡", "No gpu??? Unbelievable!"));
		
		for (auto &g : mGraphicsProcessingUnits)
		{
			if (g->Properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			{
				return *g;
			}
		}

		IM_CORE_WARN("Couldn't find a discrete physical device. Picking default GPU");
		return *mGraphicsProcessingUnits.at(0);
	}

	bool VulkanInstance::IsEnabled(const char * extension) const NOEXCEPT
	{
		return std::find_if(mEnabledExtensions.begin(), mEnabledExtensions.end(), [extension](const char *enabledExtension)
			{
				return Vulkan::Equals(extension, enabledExtension);
			}) != mEnabledExtensions.end();
	}

#if  IMMORTAL_CHECK_DEBUG
	/**
     * @brief It is what it is literally.
     */
	bool VulkanInstance::CheckValidationLayerSupport() NOEXCEPT
	{
		UINT32 layerInstanceExtensionCount;
		Vulkan::Check(vkEnumerateInstanceExtensionProperties("VK_LAYER_KHRONOS_validation", &layerInstanceExtensionCount, nullptr));

		std::vector<VkExtensionProperties> availableLayerInstanceExtension(layerInstanceExtensionCount);
		Vulkan::Check(vkEnumerateInstanceExtensionProperties("VK_LAYER_KHRONOS_validation", &layerInstanceExtensionCount, availableLayerInstanceExtension.data()));

		for (auto &ext : availableLayerInstanceExtension)
		{
			if (Vulkan::Equals(ext.extensionName, VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME))
			{
				IM_CORE_INFO(LOGB("{0} 可用. 已启用!", "{0} is available. Enabling it!"), VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);
				mEnabledExtensions.emplace_back(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);
				return true;
			}
		}
		return false;
	}
#endif
}
