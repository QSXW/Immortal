#pragma once

#include "Common.h"
#include "Descriptor.h"
#include "Device.h"

#include "Render/Texture.h"
#include "RenderContext.h"

namespace Immortal
{
namespace D3D12
{

class Texture : public SuperTexture
{
public:
    using Super = SuperTexture;

    void ConvertType(D3D12_RESOURCE_DESC &dst, Description &src)
    {
        switch (src.Format)
        {
        case Format::RGBA8:
            dst.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            break;

        case Format::BGRA8:
            dst.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
            break;

        default:
            dst.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            break;
        }
    }

public:
    Texture(RenderContext *context, const std::string &filepath, bool flip = false);

    ~Texture();

    virtual operator uint64_t() const override
    {
        return gpuDescriptorHandle.ptr;
    }

private:
    ID3D12Resource *texture{ nullptr };

    CPUDescriptor cpuDescriptorHandle;

    GPUDescriptor gpuDescriptorHandle{};

    D3D12_RESOURCE_DESC desc{};

    int descriptorIndex{ 1 };
};

}
}
