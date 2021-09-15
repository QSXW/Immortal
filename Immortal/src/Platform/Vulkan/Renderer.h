#pragma once

#include "Render/Renderer.h"

#include "Common.h"
#include "RenderContext.h"

namespace Immortal
{
namespace Vulkan
{

class Renderer : public SuperRenderer
{
public:
    using Super = SuperRenderer;

public:
    Renderer(RenderContext::Super *context);

    virtual void INIT() override;

public:
    RenderContext *context{ nullptr };
};

}
}
