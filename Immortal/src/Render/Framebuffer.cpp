#include "impch.h"
#include "Framebuffer.h"

#include "Render/Render.h"

#include "Platform/OpenGL/OpenGLFramebuffer.h"
#include "Platform/Vulkan/Framebuffer.h"
#include "Platform/D3D12/Framebuffer.h"

namespace Immortal
{

Ref<Framebuffer> Framebuffer::Create(const Framebuffer::Specification& spec)
{
    return CreateSuper<Framebuffer, OpenGLFramebuffer, Vulkan::Framebuffer, D3D12::Framebuffer>(spec);
}

}
