#include "impch.h"
#include "RendererAPI.h"

#include "Platform/OpenGL/OpenGLRenderer.h"
#include "Platform/Vulkan/Renderer.h"

namespace Immortal {

	RendererAPI::Type RendererAPI::API = RendererAPI::Type::OpenGL;

	Unique<RendererAPI> RendererAPI::Create()
	{
		switch (API)
		{
			case RendererAPI::Type::OpenGL:
				return MakeUnique<OpenGLRenderer>();

			case RendererAPI::Type::VulKan:
				return MakeUnique<Vulkan::Renderer>();
			default:
				SLASSERT(false && "RendererAPI::None is currently not supported!");
				return nullptr;
		}

		return nullptr;
	}

}