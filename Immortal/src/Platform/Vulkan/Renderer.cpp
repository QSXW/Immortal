#include "impch.h"
#include "Renderer.h"

namespace Immortal
{
namespace Vulkan
{

Renderer::Renderer(RenderContext::Super *c) : 
    context{ dcast<RenderContext *>(c) }
{

}

void Renderer::INIT()
{

}

}
}