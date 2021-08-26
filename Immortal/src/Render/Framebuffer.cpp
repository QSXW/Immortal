#include "impch.h"
#include "Framebuffer.h"

#include "Render/Renderer.h"

#include "Platform/OpenGL/OpenGLFramebuffer.h"
#include "Platform/Vulkan/Framebuffer.h"
#include "Platform/D3D12/D3D12Framebuffer.h"

namespace Immortal {

		Ref<Framebuffer> Framebuffer::Create(const Framebuffer::Specification& spec)
		{
			return InstantiateGrphicsPrimitive<Framebuffer, OpenGLFramebuffer, Vulkan::Framebuffer, D3D12Framebuffer>(spec);
		}

}