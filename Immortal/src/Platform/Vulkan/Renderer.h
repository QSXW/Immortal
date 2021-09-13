#pragma once

#include "Render/RendererAPI.h"

#include "Common.h"
#include "RenderContext.h"

namespace Immortal
{
namespace Vulkan
{

class Renderer : public RendererAPI
{
public:
    virtual void Init() override;

public:
    RenderContext *renderContext{ nullptr };
};
}
}
