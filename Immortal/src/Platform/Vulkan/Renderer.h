#pragma once

#include "Render/RendererAPI.h"

#include "vkcommon.h"

namespace Immortal
{
namespace Vulkan {
	class Renderer : public RendererAPI
	{
		virtual void Init() override;

	public:
		static inline void Set(std::vector<VkSurfaceFormatKHR> &surfacePriorityList)
		{
			SurfacePriorityList = std::move(surfacePriorityList);
		}

	public:
		static std::vector<VkSurfaceFormatKHR> SurfacePriorityList;
	};
}
}
