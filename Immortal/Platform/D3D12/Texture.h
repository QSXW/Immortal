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

public:
    Texture(RenderContext *context, const std::string &filepath, bool flip = false);

    Texture(RenderContext *context, uint32_t width, uint32_t height, const void *data, const Description &description);

    ~Texture();

    virtual operator uint64_t() const override
    {
        return gpuDescriptor.ptr;
    }

    virtual void As(Descriptor *descriptors, size_t index) override;

private:
    void InternalCreate(RenderContext *context, const Description &description, const void *data);

private:
    ID3D12Resource *texture{ nullptr };

    CPUDescriptor cpuDescriptor;

    GPUDescriptor gpuDescriptor{};

    int descriptorIndex{ 1 };
};

}
}
