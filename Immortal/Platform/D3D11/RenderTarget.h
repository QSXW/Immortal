#pragma once

#include "Common.h"
#include "Descriptor.h"
#include "Render/RenderTarget.h"
#include "Algorithm/LightArray.h"
#include "Image.h"

namespace Immortal
{
namespace D3D11
{

class Device;
class RenderTarget : public SuperRenderTarget
{
public:
    using Super = SuperRenderTarget;

    template <class T>
    struct Attachment
    {
        Ref<Image> image;
        Descriptor<T> descriptor;
    };

public:
    RenderTarget();

    RenderTarget(Device *device, const Super::Description &descrition);

    RenderTarget(Device *device, Ref<Image> image);

    ~RenderTarget();

    virtual void Map(uint32_t slot) override;

    virtual void Unmap() override;

    virtual void Resize(UINT32 width, UINT32 height) override;

    virtual operator uint64_t() const;

    RTV * const *GetViews(size_t index = 0) const
    {
		return descriptors.data();
    }

    const LightArray<Attachment<RTV>> &GetColorBuffer() const
    {
		return colorAttachments;
    }

    const Attachment<DSV> &GetDepthBuffer() const
    {
		return depthAttachment;
    }

private:
    Device *device;

    Descriptor<SRV> descriptor;

    LightArray<Attachment<RTV>> colorAttachments;

    LightArray<RTV *> descriptors;

    Attachment<DSV> depthAttachment;
};

}
}
