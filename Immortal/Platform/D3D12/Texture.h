#pragma once

#include "Render/Texture.h"

#include "Common.h"
#include "Descriptor.h"
#include "Device.h"
#include "Resource.h"
#include "RenderContext.h"

namespace Immortal
{
namespace D3D12
{

class Texture : public SuperTexture, public Resource
{
public:
    using Super = SuperTexture;

public:
    Texture(RenderContext *context, const std::string &filepath, const Description &description);

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
    Descriptor descriptor;

    int descriptorIndex{ 1 };
};

}
}
