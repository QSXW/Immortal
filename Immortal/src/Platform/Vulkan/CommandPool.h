#pragma once

#include "vkcommon.h"
#include "CommandBuffer.h"

namespace Immortal
{
namespace Vulkan
{
	class Device;

	class CommandPool
	{
	public:
		using QueueFamilyIndex = UINT32;
		using ThreadIndex      = size_t;
		using RenderFrame      = void *;
		using Handle           = VkCommandPool;
	public:
		CommandPool(Device                  &device,
			        UINT32                   queueFamilyIndex,
			        void                    *renderFrame = nullptr,
			        size_t                   threadIndex = 0,
			        CommandBuffer::ResetMode resetMode   = CommandBuffer::ResetMode::ResetPool);
		
		CommandPool(CommandPool &&other);

		~CommandPool();

		template <class T>
		T &Get()
		{
			if constexpr (std::is_same_v<T, Handle>)
			{
				return handle;
			}
			if constexpr (std::is_same_v<T, Device>)
			{
				return device;
			}
			if constexpr (std::is_same_v<T, CommandBuffer::ResetMode>)
			{
				return resetMode;
			}
			if constexpr (std::is_same_v<T, QueueFamilyIndex>)
			{
				return queueFamilyIndex;
			}
			if constexpr (std::is_same_v<T, ThreadIndex>)
			{
				return threadIndex;
			}
			if constexpr (std::is_same_v<T, RenderFrame>)
			{
				return renderFrame;
			}
		}

	private:
		Device &device;
		
		VkCommandPool handle{ VK_NULL_HANDLE };

		void *renderFrame{ nullptr };

		size_t threadIndex;

		UINT32 queueFamilyIndex{ 0 };

		std::vector<Unique<CommandBuffer>> primaryCommandBuffers;

		UINT32 activePrimaryCommandBufferCount{ 0 };

		std::vector<Unique<CommandBuffer>> secondaryCommandBuffers;

		UINT32 activeSecondaryCommandBufferCount{ 0 };

		CommandBuffer::ResetMode resetMode{ CommandBuffer::ResetMode::ResetPool };
	
	private:
		VkResult ResetCommandBuffers();
	};
}
}
