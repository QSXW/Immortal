#pragma once

#include "Render/RenderContext.h"
#include "D3D12Common.h"
#include "Device.h"

namespace Immortal
{
namespace D3D12
{
class RenderContext : public SuperRenderContext
{
public:
    using Super = SuperRenderContext;

public:
    RenderContext(const void *handle);

    RenderContext(Description &desc);

    void Initialize();

private:
    std::unique_ptr<Device> device;
};

}
}
