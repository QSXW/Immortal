#include "impch.h"
#include "Framebuffer.h"

#include "Render/Render.h"

#include "Platform/OpenGL/Framebuffer.h"
#include "Platform/Vulkan/Framebuffer.h"
#include "Platform/D3D12/Framebuffer.h"

namespace Immortal
{

Framebuffer::Framebuffer()
{

}

std::shared_ptr<Framebuffer> Framebuffer::Create(const Framebuffer::Description &desc)
{
    return CreateSuper<Framebuffer, OpenGL::Framebuffer, Vulkan::Framebuffer, D3D12::Framebuffer>(desc);
}

}
