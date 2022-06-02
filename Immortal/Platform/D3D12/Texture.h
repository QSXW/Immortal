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
        return descriptor.visible.gpu.ptr;
    }

    CPUDescriptor GetDescriptor() const
    {
        return descriptor.invisible;
    }

    virtual bool operator==(const Super &other) const override;

    virtual void As(Descriptor::Super *descriptors, size_t index) override;

    virtual void Update(const void *data, uint32_t pitchX = 0) override;

private:
    void InternalCreate(const void *data);

private:
    RenderContext *context{ nullptr };

    Format format = Format::None;

    struct {
        Descriptor visible;
        CPUDescriptor invisible;
    } descriptor;
};

}
}
