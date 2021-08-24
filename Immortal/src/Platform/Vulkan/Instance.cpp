#include "impch.h"
#include "Instance.h"

#include "Device.h"
#include "PhysicalDevice.h"
#include "RenderContext.h"

namespace Immortal
{
namespace Vulkan
{
#if IMMORTAL_CHECK_DEBUG
	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugUtilsMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		                                                              VkDebugUtilsMessageTypeFlagsEXT messageType,
		                                                              const VkDebugUtilsMessengerCallbackDataEXT *callbackData,
		                                                              void *userData)
	{
		if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		{
			Log::Warn("{0} - {1}: {2}", callbackData->messageIdNumber, callbackData->pMessageIdName, callbackData->pMessage);
		}
		else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
		{
			Log::Error("{0} - {1}: {2}", callbackData->messageIdNumber, callbackData->pMessageIdName, callbackData->pMessage);
		}

		return VK_FALSE;
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT /*type*/,
		                                                uint64_t /*object*/,  size_t /*location*/, int32_t /*message_code*/,
		                                                const char *layerPrefix, const char *message,void * /*userData*/)
	{
		if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
		{
			Log::Error("{0}: {1}", layerPrefix, message);
		}
		else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
		{
			Log::Warn("{0}: {1}", layerPrefix, message);
		}
		else if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
		{
			Log::Warn("{0}: {1}", layerPrefix, message);
		}
		else
		{
			Log::Info("{0}: {1}", layerPrefix, message);
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
				Log::Error("Validation Layer {} not found", layer);
				return false;
			}
		}

		return true;
	}

	static inline void CheckDynamicLoadedLibarary()
	{
		if (RenderContext::Status == VK_NOT_READY)
		{
			if ((RenderContext::Status = volkInitialize()) != VK_SUCCESS)
			{
				Log::Critical("Unable to initialize Vulkan Library."
					          "Please confirm your device support "
					          "and get Vulkan Installed");
				Check(RenderContext::Status);
			}
			Log::Info("Load Vulkan library with success...");
		}
	}

	static inline void VkEnableExtension(const char *message)
	{
		Log::Info("{0} is available. Enabling it!", message);
	}

	Instance::Instance(const char                                   *applicationName,
		               const std::unordered_map<const char *, bool> &requiredExtension,
		               const std::vector<const char *>              &requiredValidationLayers,
		               bool                                          headless,
		               UINT32                                        apiVersion)
	{
		CheckDynamicLoadedLibarary();

		UINT32 extensionCount;
		Check(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr));
		std::vector<VkExtensionProperties> availableExtension{ extensionCount };
		Check(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtension.data()));

#if     IMMORTAL_CHECK_DEBUG
		bool debugUtils = false;
#endif
		bool headlessExtension = false;
		for (auto &ext : availableExtension)
		{
#if			IMMORTAL_CHECK_DEBUG
			if (Equals(ext.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
			{
				debugUtils = true;
				VkEnableExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
				enabledExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			}
#endif
			if (headless && Equals(ext.extensionName, VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME))
			{
				headlessExtension = true;
				VkEnableExtension(VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME);
				enabledExtensions.emplace_back(VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME);
			}

			/**
		     * @brief VK_KHR_get_physical_device_properties2 is a prerequisite of
			 *        VK_KHR_performance_query which will be used for stats gathering
			 *        where available.
			 * 
			 */
			if (Equals(ext.extensionName, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME))
			{
				VkEnableExtension(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
				enabledExtensions.emplace_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
			}
		}
		if (headless && !headlessExtension)
		{
			Log::Warn("{0} is not available. Disabling swapchain creation", VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME);
		}
		else
		{
			enabledExtensions.emplace_back(VK_KHR_SURFACE_EXTENSION_NAME);
		}

#if     IMMORTAL_CHECK_DEBUG
		if (!debugUtils)
		{
			enabledExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		}
#endif

		for (auto &ext : requiredExtension)
		{
			auto extensionName = ext.first;
			auto isOptional    = ext.second;

			if (std::find_if(availableExtension.begin(), availableExtension.end(), [&extensionName](VkExtensionProperties &availableExtension)
				{
					return Equals(availableExtension.extensionName, extensionName);
				}) == availableExtension.end())
			{
				if (isOptional)
				{
					Log::Warn("Optional instance extension {0} not available, some features may be disabled", extensionName);
				}
				else
				{
					Log::Error("Required instance extension {0} not available, cannot run", extensionName);
					Log::Error("Required instance extensions are missing.");
				}
			}
			else
			{
				enabledExtensions.emplace_back(extensionName);
			}
		}

		UINT32 layerCount;
		Vulkan::Check(vkEnumerateInstanceLayerProperties(&layerCount, nullptr));

		std::vector<VkLayerProperties> supportedValidationLayers(layerCount);
		Vulkan::Check(vkEnumerateInstanceLayerProperties(&layerCount, supportedValidationLayers.data()));

		if (ValidateLayers(requiredValidationLayers, supportedValidationLayers))
		{
			Log::Info("Enabled Validation Layers: ");
			for (const auto &layer : requiredValidationLayers)
			{
				Log::Debug("  \t{0}", layer);;
			}
		}
		else
		{
			Log::Error("Required validation layers are missing.");
		}

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
		instanceInfo.enabledExtensionCount   = static_cast<UINT32>(enabledExtensions.size());
		instanceInfo.ppEnabledExtensionNames = enabledExtensions.data();
		instanceInfo.enabledLayerCount       = static_cast<UINT32>(requiredValidationLayers.size());
		instanceInfo.ppEnabledLayerNames     = requiredValidationLayers.data();

#if     IMMORTAL_CHECK_DEBUG
		VkDebugUtilsMessengerCreateInfoEXT debugUtilsCreateInfo = { VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
		VkDebugReportCallbackCreateInfoEXT debugReportCreateInfo = { VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT };

		if (debugUtils)
		{
			debugUtilsCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
			debugUtilsCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			debugUtilsCreateInfo.pfnUserCallback = Vulkan::DebugUtilsMessengerCallback;

			instanceInfo.pNext = &debugUtilsCreateInfo;
		}
		else
		{
			debugReportCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
			debugReportCreateInfo.pfnCallback = Vulkan::DebugCallback;

			instanceInfo.pNext = &debugReportCallback;
		}
#endif

		Vulkan::Check(vkCreateInstance(&instanceInfo, nullptr, &handle));
		volkLoadInstance(handle);

#if     IMMORTAL_CHECK_DEBUG
		if (debugUtils)
		{
			SLASSERT(vkCreateDebugUtilsMessengerEXT != nullptr && "");
			Vulkan::Check(vkCreateDebugUtilsMessengerEXT(handle, &debugUtilsCreateInfo, nullptr, &debugUtilsMessengers));
		}
		else
		{
			SLASSERT(vkCreateDebugReportCallbackEXT != nullptr && "");
			Vulkan::Check(vkCreateDebugReportCallbackEXT(handle, &debugReportCreateInfo, nullptr, &debugReportCallback));
		}
#endif
		QueryPhysicalDevice();
	}

	Instance::Instance(VkInstance instance)
		: handle(instance)
	{
		SLASSERT(handle != VK_NULL_HANDLE && "Instance not valid");
		QueryPhysicalDevice();
	}

	Instance::~Instance()
	{
#if defined( Debug ) || defined( _DEBUG )
		if (debugUtilsMessengers)
		{
			vkDestroyDebugUtilsMessengerEXT(handle, debugUtilsMessengers, nullptr);
		}
		if (debugReportCallback)
		{
			vkDestroyDebugReportCallbackEXT(handle, debugReportCallback, nullptr);
		}
#endif
		if (handle)
		{
			IfNotNullThen<VkInstance>(vkDestroyInstance, handle, nullptr);
		}
	}

	void Instance::QueryPhysicalDevice()
	{
		UINT32 count{ 0 };
		Check(vkEnumeratePhysicalDevices(handle, &count, nullptr));

		SLASSERT(count >= 1 && "Couldn't find a physical device that supports Vulkan.");

		std::vector<VkPhysicalDevice> physicalDevices{ count };
		Check(vkEnumeratePhysicalDevices(handle, &count, physicalDevices.data()));

		for (auto &pd : physicalDevices)
		{
			this->physicalDevices.emplace_back(MakeUnique<PhysicalDevice>(*this, pd));
		}
	}

	PhysicalDevice &Instance::SuitablePhysicalDevice()
	{
		SLASSERT(!physicalDevices.empty() && "There is no GPU on this device.");

		for (auto &pd : physicalDevices)
		{
			if (pd->Properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			{
				return *pd;
			}
		}

		Log::Warn("Couldn't find a discrete physical device. Picking default GPU");
		return *physicalDevices.at(0);
	}

	bool Instance::CheckValidationLayerSupport()
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
				enabledExtensions.emplace_back(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);
				return true;
			}
		}
		return false;
	}
}
}
