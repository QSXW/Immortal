#include "impch.h"
#include "Framebuffer.h"

#include "Render/Renderer.h"

#include "Platform/OpenGL/OpenGLFramebuffer.h"
#include "Platform/Vulkan/VulkanFramebuffer.h"
#include "Platform/D3D12/D3D12Framebuffer.h"

namespace Immortal {

		Ref<Framebuffer> Framebuffer::Create(const Framebuffer::Specification& spec)
		{
			return InstantiateGrphicsPrimitive<Framebuffer, OpenGLFramebuffer, VulkanFramebuffer, D3D12Framebuffer>(spec);
		}

}