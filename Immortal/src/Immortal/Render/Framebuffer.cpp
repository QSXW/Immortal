#include "impch.h"
#include "Framebuffer.h"

#include "Immortal/Render/Renderer.h"

#include "Immortal/Platform/OpenGL/OpenGLFramebuffer.h"
#include "Immortal/Platform/Vulkan/VulkanFramebuffer.h"
#include "Immortal/Platform/D3D12/D3D12Framebuffer.h"

namespace Immortal {

		Ref<Framebuffer> Framebuffer::Create(const Framebuffer::Specification& spec)
		{
			return InstantiateGrphicsPrimitive<Framebuffer, OpenGLFramebuffer, VulkanFramebuffer, D3D12Framebuffer>(spec);
		}

}