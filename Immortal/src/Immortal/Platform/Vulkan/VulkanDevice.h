#pragma once

#include <map>

#include "VulkanCommon.h"
#include "ImmortalCore.h"

#include "VulkanInstance.h"
#include "VulkanQueue.h"

namespace Immortal
{
	/**
	  * @brief A physical Device usually represents a single complete implementation of Vulkan
	  * (excluding instance-level functionality) available to the host, of which there are a finite number.
	  *
	  */
	class VulkanPhysicalDevice
	{
	public:
		VulkanPhysicalDevice() = default;
		VulkanPhysicalDevice(VulkanInstance &instance, VkPhysicalDevice physicalDevice);

		/**
		 * @brief Requests a third party extension to be used by the framework
		 *
		 *        To have the features enabled, this function must be called before the logical device
		 *        is created. To do this request sample specific features inside
		 *        VulkanSample::RequestExtensionFeatures(VulkanPhysicalDevice &gpu).
		 *
		 *        If the feature extension requires you to ask for certain features to be enabled, you can
		 *        modify the struct returned by this function, it will propegate the changes to the logical
		 *        device.
		 * @param type The VkStructureType for the extension you are requesting
		 * @returns The extension feature struct
		 */
		template <typename T>
		T &RequestExtensionFeatures(VkStructureType type)
		{
			// We cannot request extension features if the physical device properties 2 instance extension isnt enabled
			if (!Instance.IsEnabled(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME))
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

			auto vkGetPhysicalDeviceFeatures2KHR = (PFN_vkGetPhysicalDeviceFeatures2KHR)vkGetInstanceProcAddr(Instance.Handle(),"vkGetPhysicalDeviceFeatures2KHR");

			vkGetPhysicalDeviceFeatures2KHR(mHandle, &physicalDeviceFeatures);

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

		/**
		 * @brief Not all physical devices will include WSI support. Within a physical device, not all queue families
		 *	will support presentation. WSI support and compatibility can be determined in a platform-neutral
		 *	manner (which determines support for presentation to a particular surface object) and additionally
		 *	may be determined in platform-specific manners (which determine support for presentation on the
		 *	specified physical device but do not guarantee support for presentation to a particular surface
		 *	object).
		 * 
		 * *
		 */
		VkBool32 IsPresentSupported(VkSurfaceKHR surface, UINT32 queueFamilyIndex) NOEXCEPT;

		DefineGetHandleFunc(VkPhysicalDevice)

	private:
		VkPhysicalDevice mHandle{ VK_NULL_HANDLE };

	public:
		VulkanInstance &Instance;

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


	/**
	  * @brief Once Vulkan is initialized, devices and queues are the primary objects used
	  * to interact with a Vulkan implementation.
	  * 
	  * Vulkan Device, aka logical devices, represents an instance of that implementation with
	  * its own state and resources independent of other logical devices.
	  * Each device exposes a number of queue families each having one or more queues. All queues
	  * in a queue family support the same operations.
	  */
	class VulkanDevice
	{
	public:
		VulkanDevice() = default;
		VulkanDevice(VulkanPhysicalDevice &gpu, VkSurfaceKHR surface, std::unordered_map<const char *, bool> requestedExtensions = {});
		~VulkanDevice();

		DefineGetHandleFunc(VkDevice)

		UINT32 QueueFailyIndex(VkQueueFlagBits queueFlag);

		void CheckExtensionSupported() NOEXCEPT;

		bool IsExtensionSupport(const char *extension) NOEXCEPT;

		bool IsEnabled(const char * extension) const NOEXCEPT;

	private:
		VulkanPhysicalDevice &mGraphicsProcessingUnit;

		VkSurfaceKHR mSurface{ VK_NULL_HANDLE };

		VkDevice mHandle{ VK_NULL_HANDLE };

		std::vector<VkExtensionProperties> mDeviceExtensions;

		std::vector<const char *> mEnabledExtensions{};

		VmaAllocator mMemoryAllocator{ VK_NULL_HANDLE };

		std::vector<std::vector<VulkanQueue>> mQueues;

		bool mHasGetMemoryRequirements = false;
		bool mHasDedicatedAllocation = false;
		bool mHasBufferDeviceAddressName = false;

	};
}

