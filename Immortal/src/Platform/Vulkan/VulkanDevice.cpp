#include "impch.h"
#include "VulkanDevice.h"

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include "VulkanCommon.h"
#include "VulkanPhysicalDevice.h"

namespace Immortal
{
	VulkanDevice::VulkanDevice(VulkanPhysicalDevice &gpu, VkSurfaceKHR surface, std::unordered_map<const char*, bool> requestedExtensions)
		: mGraphicsProcessingUnit(gpu),
		  mSurface(surface)
	{
		IM_CORE_INFO("Selected GPU: {0}", gpu.Properties.deviceName);

		UINT32 propsCount = static_cast<UINT32>(gpu.QueueFamilyProperties.size());

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos(propsCount, { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO });
		std::vector<std::vector<float>>      queueProps(propsCount);

		for (UINT32 index = 0; index < propsCount; index++)
		{
			const VkQueueFamilyProperties &prop = gpu.QueueFamilyProperties[index];
			if (gpu.HighPriorityGraphicsQueue)
			{
				UINT32 graphicsQueueFamily = QueueFailyIndex(VK_QUEUE_GRAPHICS_BIT);
				if (graphicsQueueFamily == index)
				{
					queueProps[index].reserve(prop.queueCount);
					queueProps[index].push_back(1.0f);
					for (UINT32 i = 1; i < prop.queueCount; i++)
					{
						queueProps[index].push_back(0.5f);
					}
				}
			}
			else
			{
				queueProps[index].resize(prop.queueCount, 0.5f);
			}

			auto &createInfo = queueCreateInfos[index];

			createInfo.pNext            = nullptr;
			createInfo.queueFamilyIndex = index; // This index corresponds to the index of an element of the pQueueFamilyProperties array that was returned by vkGetPhysicalDeviceQueueFamilyProperties
			createInfo.queueCount       = prop.queueCount; // must be greater than 0
			createInfo.pQueuePriorities = queueProps[index].data(); //e a valid pointer to an array of queueCount float value, must be between 0.0 and 1.0 inclusive
		}

		// Check extensions to enable Vma dedicated Allocation
		UINT32 deviceExtensionCount;
		Vulkan::Check(vkEnumerateDeviceExtensionProperties(gpu.Handle(), nullptr, &deviceExtensionCount, nullptr));

		mDeviceExtensions.resize(deviceExtensionCount);
		Vulkan::Check(vkEnumerateDeviceExtensionProperties(gpu.Handle(), nullptr, &deviceExtensionCount, mDeviceExtensions.data()));

#if     IMMORTAL_CHECK_DEBUG
		if (!mDeviceExtensions.empty())
		{
			IM_CORE_INFO("Device supports the following extensions: ");
			for (auto &ext : mDeviceExtensions)
			{
				IM_CORE_INFO("  \t{0}", ext.extensionName);
			}
		}
#endif
		
		// @required
		CheckExtensionSupported();

		// @required
		// Check that extensions are supported before creating the device
		std::vector<const char *> unsupportedExtensions{};
		for (auto &ext : requestedExtensions)
		{
			if (IsExtensionSupport(ext.first))
			{
				mEnabledExtensions.emplace_back(ext.first);
			}
			else
			{
				unsupportedExtensions.emplace_back(ext.first);
			}
		}

		if (!mEnabledExtensions.empty())
		{
			IM_CORE_INFO("Device supports the following requested extensions:");
			for (auto &ext : mEnabledExtensions)
			{
				IM_CORE_INFO("  \t{0}", ext);
			}
		}

		if (!unsupportedExtensions.empty())
		{
			for (auto &ext : unsupportedExtensions)
			{
				auto isOptional = requestedExtensions[ext];
				if (isOptional)
				{
					IM_CORE_WARN("Optional device extension {0} not available. Some features may be disabled", ext);
				}
				else
				{
					IM_CORE_ERROR("Required device extension {0} not available. Stop running!", ext);
					Vulkan::Check(VK_ERROR_EXTENSION_NOT_PRESENT);
				}
			}
		}
		
		// @required
		// @info flags is reserved for future use.
		// @warn enableLayer related is deprecated and ignored
		VkDeviceCreateInfo createInfo{};
		createInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pNext                   = gpu.LastRequestedExtensionFeature;
		createInfo.queueCreateInfoCount    = Vulkan::ToUINT32(queueCreateInfos.size());
		createInfo.pQueueCreateInfos       = queueCreateInfos.data();
		createInfo.enabledExtensionCount   = Vulkan::ToUINT32(mEnabledExtensions.size());
		createInfo.ppEnabledExtensionNames = mEnabledExtensions.data();
		createInfo.pEnabledFeatures        = &gpu.RequestedFeatures;

		Vulkan::Check(vkCreateDevice(gpu.Handle(), &createInfo, nullptr, &handle));

		mQueues.resize(propsCount);
		for (UINT32 queueFamilyIndex = 0U; queueFamilyIndex < propsCount; queueFamilyIndex++)
		{
			const auto &queueFamilyProps = gpu.QueueFamilyProperties[queueFamilyIndex];
			VkBool32 presentSupported = gpu.IsPresentSupported(mSurface, queueFamilyIndex);

			for (UINT32 queueIndex = 0U; queueIndex < queueFamilyProps.queueCount; queueIndex++)
			{
				mQueues[queueFamilyIndex].emplace_back(*this, queueFamilyIndex, queueFamilyProps, presentSupported, queueIndex);
			}
		}

		// @required
		VmaVulkanFunctions vmaVulkanFunc{};
		vmaVulkanFunc.vkAllocateMemory = vkAllocateMemory;
		vmaVulkanFunc.vkBindBufferMemory = vkBindBufferMemory;
		vmaVulkanFunc.vkBindImageMemory = vkBindImageMemory;
		vmaVulkanFunc.vkCreateBuffer = vkCreateBuffer;
		vmaVulkanFunc.vkCreateImage = vkCreateImage;
		vmaVulkanFunc.vkDestroyBuffer = vkDestroyBuffer;
		vmaVulkanFunc.vkDestroyImage = vkDestroyImage;
		vmaVulkanFunc.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
		vmaVulkanFunc.vkFreeMemory = vkFreeMemory;
		vmaVulkanFunc.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
		vmaVulkanFunc.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
		vmaVulkanFunc.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
		vmaVulkanFunc.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
		vmaVulkanFunc.vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges;
		vmaVulkanFunc.vkMapMemory = vkMapMemory;
		vmaVulkanFunc.vkUnmapMemory = vkUnmapMemory;
		vmaVulkanFunc.vkCmdCopyBuffer = vkCmdCopyBuffer;

		// @required
		VmaAllocatorCreateInfo allocatorInfo{};
		allocatorInfo.physicalDevice = mGraphicsProcessingUnit.Handle();
		allocatorInfo.device = handle;
		allocatorInfo.instance = mGraphicsProcessingUnit.Instance.Handle();

		if (mHasBufferDeviceAddressName)
		{
			allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
		}

		if (mHasGetMemoryRequirements && mHasDedicatedAllocation)
		{
			allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT;

			auto vkGetBufferMemoryRequirements2KHR = (PFN_vkGetBufferMemoryRequirements2KHR)vkGetDeviceProcAddr(handle, "vkGetBufferMemoryRequirements2KHR");
			auto vkGetImageMemoryRequirements2KHR = (PFN_vkGetImageMemoryRequirements2KHR)vkGetDeviceProcAddr(handle, "vkGetImageMemoryRequirements2KHR");
			vmaVulkanFunc.vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2KHR;
			vmaVulkanFunc.vkGetImageMemoryRequirements2KHR = vkGetImageMemoryRequirements2KHR;
		}

		allocatorInfo.pVulkanFunctions = &vmaVulkanFunc;
		Vulkan::Check(vmaCreateAllocator(&allocatorInfo, &mMemoryAllocator));

		// @required Command Pool

		// @required Fence Pool

	}

	UINT32 VulkanDevice::QueueFailyIndex(VkQueueFlagBits queueFlag)
	{
		const auto &queueFamilyProperties = mGraphicsProcessingUnit.QueueFamilyProperties;

		// @required Compute Queue
		if (queueFlag & VK_QUEUE_COMPUTE_BIT)
		{
			for (UINT32 i = 0; i < Vulkan::ToUINT32(queueFamilyProperties.size()); i++)
			{
				if ((queueFamilyProperties[i].queueFlags & queueFlag) && ((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0))
				{
					return i;
					break;
				}
			}
		}

		// @required Graphics Queue
		if (queueFlag & VK_QUEUE_TRANSFER_BIT)
		{
			for (UINT32 i = 0; i < Vulkan::ToUINT32(queueFamilyProperties.size()); i++)
			{
				if ((queueFamilyProperties[i].queueFlags & queueFlag) && ((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) && ((queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) == 0))
				{
					return i;
					break;
				}
			}
		}

		for (UINT32 i = 0; i < Vulkan::ToUINT32(queueFamilyProperties.size()); i++)
		{
			if (queueFamilyProperties[i].queueFlags & queueFlag)
			{
				return i;
				break;
			}
		}

		IM_CORE_ERROR("Counld not find a matching queue family index");
		return 0;
	}


	/**
	 * @breif Queues are created along with a logical device during vkCreateDevice. All queues associated with a
     *  logical device are destroyed when vkDestroyDevice is called on that device
	 */
	VulkanDevice::~VulkanDevice()
	{
		static auto DestroyVmaAllocator = [](VmaAllocator memoryAllocator)
		{
			VmaStats stats;
			vmaCalculateStats(memoryAllocator, &stats);
			IM_CORE_INFO("Total device memory leaked: {0} bytes.", stats.total.usedBytes);

			vmaDestroyAllocator(memoryAllocator);
		};

		Vulkan::IfNotNullThen<VmaAllocator, DestroyVmaAllocator>(mMemoryAllocator);
		Vulkan::IfNotNullThen(vkDestroyDevice, handle);
	}

	bool VulkanDevice::IsExtensionSupport(const char * extension) NOEXCEPT
	{
		return std::find_if(mDeviceExtensions.begin(), mDeviceExtensions.end(), [extension](auto &deviceExtension)
			{
				return Vulkan::Equals(deviceExtension.extensionName, extension);
			}) != mDeviceExtensions.end();
	}

	bool VulkanDevice::IsEnabled(const char * extension) const NOEXCEPT
	{
		return std::find_if(mEnabledExtensions.begin(), mEnabledExtensions.end(), [extension](const char *enabledExtension)
			{
				return Vulkan::Equals(extension, enabledExtension);
			}) != mEnabledExtensions.end();
	}

	void VulkanDevice::CheckExtensionSupported() NOEXCEPT
	{
		bool hasPerformanceQuery = false;
		bool hasHostQueryReset   = false;

		for (auto &e : mDeviceExtensions)
		{
			if (Vulkan::Equals(e.extensionName, "VK_KHR_get_memory_requirements2"))
			{
				mHasGetMemoryRequirements = true;
			}
			if (Vulkan::Equals(e.extensionName, "VK_KHR_dedicated_allocation"))
			{
				mHasDedicatedAllocation = true;
			}
			if (Vulkan::Equals(e.extensionName, "VK_KHR_performance_query"))
			{
				hasPerformanceQuery = true;
			}
			if (Vulkan::Equals(e.extensionName, "VK_EXT_host_query_reset"))
			{
				hasHostQueryReset = true;
			}
			if (Vulkan::Equals(e.extensionName, VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME) && IsEnabled(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME))
			{
				mHasBufferDeviceAddressName = true;
			}
		}

		if (mHasGetMemoryRequirements && mHasDedicatedAllocation)
		{
			mEnabledExtensions.emplace_back("VK_KHR_get_memory_requirements2");
			mEnabledExtensions.emplace_back("VK_KHR_dedicated_allocation");

			IM_CORE_INFO("Dedicated Allocation enabled");
		}

		if (hasPerformanceQuery && hasHostQueryReset)
		{
			auto &perfCounterFeatures       = mGraphicsProcessingUnit.RequestExtensionFeatures<VkPhysicalDevicePerformanceQueryFeaturesKHR>(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PERFORMANCE_QUERY_FEATURES_KHR);
			auto &host_query_reset_features = mGraphicsProcessingUnit.RequestExtensionFeatures<VkPhysicalDeviceHostQueryResetFeatures>(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES);
			IM_CORE_INFO("Performance query enabled");
		}
	}

	const VulkanQueue &VulkanDevice::SuitableGraphicsQueue()
	{
		for (UINT32 familyIndex = 0U; familyIndex < mQueues.size(); familyIndex++)
		{
			VulkanQueue &firstQueue = mQueues[familyIndex][0];

			UINT32 queueCount = firstQueue.Properties().queueCount;

			if (firstQueue.IsPresentSupported() && 0 < queueCount)
			{
				return mQueues[familyIndex][0];
			}
		}

		return FindQueueByFlags(VK_QUEUE_GRAPHICS_BIT, 0);
	}


	const VulkanQueue &VulkanDevice::FindQueueByFlags(VkQueueFlags flags, UINT32 queueIndex)
	{
		for (uint32_t familyIndex = 0U; familyIndex < mQueues.size(); ++familyIndex)
		{
			VulkanQueue &firstQueue = mQueues[familyIndex][0];

			VkQueueFlags queueFlags = firstQueue.Properties().queueFlags;
			uint32_t     queueCount = firstQueue.Properties().queueCount;

			if (((queueFlags & flags) == flags) && queueIndex < queueCount)
			{
				return mQueues[familyIndex][queueIndex];
			}
		}

		IM_CORE_ERROR("Queue not found");
		return Utils::NullValue<VulkanQueue>();
	}
}
