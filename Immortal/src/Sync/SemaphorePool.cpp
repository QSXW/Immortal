#include "impch.h"

#include "SemaphorePool.h"
#include "Platform/Vulkan/SemaphorePool.h"

namespace Immortal
{
    template <class ... Args>
	static inline Ref<SemaphorePool> SemaphorePool::Create(Args&& ... args)
	{
		if (type == RendererAPI::Type::VulKan)
		{
			return CreateRef<Vulkan::SemaphorePool>(std::forward<Args>(args)...);
		}
	}
}
