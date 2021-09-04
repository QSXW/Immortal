#pragma once

#include "Render/RenderContext.h"
#include "D3D12Common.h"
#include "Device.h"

namespace Immortal
{
namespace D3D12
{
class RenderContext : public Immortal::RenderContext
{
public:
    using Super = Immortal::RenderContext;

public:
    RenderContext(const void *handle);

    RenderContext(Description &desc);

    void Initialize();

private:
    Unique<Device> device;
};

}}
