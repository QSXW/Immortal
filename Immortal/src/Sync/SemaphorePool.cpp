#include "impch.h"

#include "SemaphorePool.h"
#include "Platform/Vulkan/SemaphorePool.h"

namespace Immortal
{
template <class ... Args>
static inline std::shared_ptr<SemaphorePool> SemaphorePool::Create(Args&& ... args)
{
	if (type == RendererAPI::Type::VulKan)
	{
		return std::make_shared<Vulkan::SemaphorePool>(std::forward<Args>(args)...);
	}
}
}
