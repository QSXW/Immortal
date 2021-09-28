#pragma once

#include "Render/RenderContext.h"
#include "D3D12Common.h"

#include "Device.h"
#include "Swapchain.h"

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

    ~RenderContext();

    void INIT();

private:
    Super::Description desc{};

    HWND hWnd{ nullptr };

    HMODULE hModule{ nullptr };

    ComPtr<IDXGIFactory4> factory{ nullptr };

    std::unique_ptr<Device> device;

    std::unique_ptr<Queue> queue;

    std::unique_ptr<Swapchain> swapchain;
};

}
}
