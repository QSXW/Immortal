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
        return descriptor.gpu.ptr;
    }

    virtual void As(Descriptor::Super *descriptors, size_t index) override;

private:
    void InternalCreate(RenderContext *context, const Description &description, const void *data);

private:
    ID3D12Resource *texture{ nullptr };

    Descriptor descriptor;

    int descriptorIndex{ 1 };
};

}
}
