#pragma once

#include "ImmortalCore.h"
#include "vkcommon.h"

#include "Instance.h"

namespace Immortal
{
namespace Vulkan
{
	class PhysicalDevice
	{
	public:
		PhysicalDevice() = default;
		PhysicalDevice(Instance &instance, VkPhysicalDevice physicalDevice);

		/**
		 * @brief Requests a third party extension to be used by the framework
		 *
		 *    To have the features enabled, this function must be called before the logical device
		 *    is created. To do this request sample specific features inside
		 *    VulkanSample::RequestExtensionFeatures(PhysicalDevice &gpu).
		 *
		 *    If the feature extension requires you to ask for certain features to be enabled, you can
		 *    modify the struct returned by this function, it will propegate the changes to the logical
		 *    device.
		 * 
		 * @param type The VkStructureType for the extension you are requesting
		 * @returns The extension feature struct
		 */
		template <typename T>
		T &RequestExtensionFeatures(VkStructureType type)
		{
			// We cannot request extension features if the physical device properties 2 instance extension isnt enabled
			if (!instance.IsEnabled(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME))
			{
				IM_CORE_ERROR("Couldn't request feature from device as " + std::string(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME) + " isn't enabled!");
			}

			// If the type already exists in the map, return a casted pointer to get the extension feature struct
			auto extensionFeaturesIterator = ExtensionFeatures.find(type);
			if (extensionFeaturesIterator != ExtensionFeatures.end())
			{
				return *static_cast<T *>(extensionFeaturesIterator->second.get());
			}

			// Get the extension feature
			VkPhysicalDeviceFeatures2KHR physicalDeviceFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2_KHR };
			T                            extension{ type };
			physicalDeviceFeatures.pNext = &extension;

			auto vkGetPhysicalDeviceFeatures2KHR = (PFN_vkGetPhysicalDeviceFeatures2KHR)vkGetInstanceProcAddr(instance.Handle(), "vkGetPhysicalDeviceFeatures2KHR");

			vkGetPhysicalDeviceFeatures2KHR(handle, &physicalDeviceFeatures);

			// Insert the extension feature into the extension feature map so its ownership is held
			ExtensionFeatures.insert({ type, CreateRef<T>(extension) });

			// Pull out the dereferenced void pointer, we can assume its type based on the template
			auto *pExtension = static_cast<T *>(ExtensionFeatures.find(type)->second.get());

			// If an extension feature has already been requested, we shift the linked list down by one
			// Making this current extension the new base pointer
			if (LastRequestedExtensionFeature)
			{
				pExtension->pNext = LastRequestedExtensionFeature;
			}
			LastRequestedExtensionFeature = pExtension;

			return *pExtension;
		}

		VkBool32 IsPresentSupported(VkSurfaceKHR surface, UINT32 queueFamilyIndex) NOEXCEPT;

		DefineGetHandleFunc(VkPhysicalDevice)

	public:
		template <class T>
		T &Get()
		{
			if constexpr (std::is_same_v<T, Instance>)
			{
				return instance;
			}
		}

	private:
		VkPhysicalDevice handle{ VK_NULL_HANDLE };

	public:
		Instance &instance;

		VkPhysicalDeviceFeatures Features;

		VkPhysicalDeviceProperties Properties;

		VkPhysicalDeviceMemoryProperties MemoryProperties;

		std::vector<VkQueueFamilyProperties> QueueFamilyProperties{};

		// The features that will be requested to be enabled in the logical device
		VkPhysicalDeviceFeatures RequestedFeatures{};

		void *LastRequestedExtensionFeature{ nullptr };

		std::map<VkStructureType, Ref<void>> ExtensionFeatures;

		bool HighPriorityGraphicsQueue{};
	};
}
}
