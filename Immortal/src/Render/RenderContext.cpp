#include "impch.h"
#include "RenderContext.h"

#include "Renderer.h"
#include "Platform/OpenGL/OpenGLRenderContext.h"
#include "Platform/Vulkan/VulkanRenderContext.h"

namespace Immortal {

	Unique<RenderContext> RenderContext::Create(Description &desc)
	{
		switch (Renderer::API())
		{
		case RendererAPI::Type::OpenGL:
			return MakeUnique<OpenGLRenderContext>(desc);

		case RendererAPI::Type::VulKan:
			return MakeUnique<VulkanRenderContext>(desc);

		default:
			IM_CORE_ERROR("Not support api");
			return nullptr;
		}
	}
}
