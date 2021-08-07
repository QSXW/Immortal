#pragma once

#define VOLK_IMPLEMENTATION
#include "volk.h"
#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"
#include "ImmortalCore.h"

namespace Immortal
{
#define DefineGetHandleFunc(Type) Type &Handle() NOEXCEPT { return mHandle; }

	namespace Vulkan
	{
		template <class T>
		constexpr inline UINT32 ToUINT32(T x)
		{
			return static_cast<UINT32>(x);
		}


		template <class T, void (*F)(VkDevice, T, const VkAllocationCallbacks *)>
		constexpr inline void IfNotNullThen(VkDevice device, T handle)
		{
			if (handle != VK_NULL_HANDLE)
			{
				F(device, handle, nullptr);
			}
		}

		template <class T, void (*F)(T, const VkAllocationCallbacks *)>
		constexpr inline void IfNotNullThen(T handle)
		{
			if (handle != VK_NULL_HANDLE)
			{
				F(handle, nullptr);
			}
		}

		template <class T, void (*F)(T)>
		constexpr inline void IfNotNullThen(T handle)
		{
			if (handle != VK_NULL_HANDLE)
			{
				F(handle);
			}
		}

		inline const char *ErrorToString(VkResult err)
		{
#define XX(x) case x: return #x;
			switch (err)
			{
				XX(VK_SUCCESS)
				XX(VK_NOT_READY)
				XX(VK_TIMEOUT)
				XX(VK_EVENT_SET)
				XX(VK_EVENT_RESET)
				XX(VK_INCOMPLETE)
				XX(VK_ERROR_OUT_OF_HOST_MEMORY)
				XX(VK_ERROR_OUT_OF_DEVICE_MEMORY)
				XX(VK_ERROR_INITIALIZATION_FAILED)
				XX(VK_ERROR_DEVICE_LOST)
				XX(VK_ERROR_MEMORY_MAP_FAILED)
				XX(VK_ERROR_LAYER_NOT_PRESENT)
				XX(VK_ERROR_EXTENSION_NOT_PRESENT)
				XX(VK_ERROR_FEATURE_NOT_PRESENT)
				XX(VK_ERROR_INCOMPATIBLE_DRIVER)
				XX(VK_ERROR_TOO_MANY_OBJECTS)
				XX(VK_ERROR_FORMAT_NOT_SUPPORTED)
				XX(VK_ERROR_FRAGMENTED_POOL)
				XX(VK_ERROR_UNKNOWN)
				XX(VK_ERROR_OUT_OF_POOL_MEMORY)
				XX(VK_ERROR_INVALID_EXTERNAL_HANDLE)
				XX(VK_ERROR_FRAGMENTATION)
				XX(VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS)
				XX(VK_ERROR_SURFACE_LOST_KHR)
				XX(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR)
				XX(VK_SUBOPTIMAL_KHR)
				XX(VK_ERROR_OUT_OF_DATE_KHR)
				XX(VK_ERROR_INCOMPATIBLE_DISPLAY_KHR)
				XX(VK_ERROR_VALIDATION_FAILED_EXT)
				XX(VK_ERROR_INVALID_SHADER_NV)
				XX(VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT)
				XX(VK_ERROR_NOT_PERMITTED_EXT)
				XX(VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT)
				XX(VK_THREAD_IDLE_KHR)
				XX(VK_THREAD_DONE_KHR)
				XX(VK_OPERATION_DEFERRED_KHR)
				XX(VK_OPERATION_NOT_DEFERRED_KHR)
				XX(VK_PIPELINE_COMPILE_REQUIRED_EXT)
			}
			return "";
		}

		const char *ToString(VkFormat format);
		const char *ToString(VkPresentModeKHR presentMode);
		const char *ToString(VkImageUsageFlagBits imageUsage);
		const char *ToString(VkSurfaceTransformFlagBitsKHR transform);
		const char *ToString(VkCompositeAlphaFlagBitsKHR compositeAlpha);


		inline const std::string ToString(VkSurfaceFormatKHR surfaceFormat)
		{
			std::string surfaceFormatString = std::string(ToString(surfaceFormat.format)) + ", ";

			switch (surfaceFormat.colorSpace)
			{
			case VK_COLORSPACE_SRGB_NONLINEAR_KHR:
				surfaceFormatString += "VK_COLORSPACE_SRGB_NONLINEAR_KHR";
				break;
			default:
				surfaceFormatString += "UNKNOWN COLOR SPACE";
			}
			return surfaceFormatString;
		}
		
#if defined( IM_DEBUG )
		/** @brief Helper macro to test the result of Vulkan calls which can return an error.
		 *
		 */
		inline void Check(VkResult err)
		{
			auto msg = ErrorToString(err);
			if (err)
			{
				IM_CORE_ERROR("Detected Vulkan error: {0}", msg);
				abort();
			}
		}

#else
		constexpr inline void Check(VkResult err) { }
#endif

		static inline bool Equals(const char *str1, const char *str2)
		{
			return (strcmp(str1, str2) == 0);
		}


		/**
		 * @brief Helper function to determine a suitable supported depth format based on a priority list
		 * @param physicalDevice The physical device to check the depth formats against
		 * @param depthOnly (Optional) Wether to include the stencil component in the format or not
		 * @param depthFormatPriorities (Optional) The list of depth formats to prefer over one another
		 *		  By default we start with the highest precision packed format
		 * @return The valid suited depth format
		 */
		VkFormat SuitableDepthFormat(VkPhysicalDevice physicalDevice,
			                         bool depthOnly = false,
			                         const std::vector<VkFormat> &depthFormatPriorities = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D16_UNORM });

		inline bool IsDepthOnlyFormat(VkFormat format)
		{
			return format == VK_FORMAT_D16_UNORM || format == VK_FORMAT_D32_SFLOAT;
		}
	}

}




