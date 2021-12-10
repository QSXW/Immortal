#pragma once

#include "Render/RenderTarget.h"

#include "Common.h"

namespace Immortal
{
namespace D3D12
{

class RenderTarget : public SuperRenderTarget
{
public:
    using Super = SuperRenderTarget;

public:
    RenderTarget();

    RenderTarget(const Super::Description &descrition);

    ~RenderTarget();

    virtual void Map(uint32_t slot) override;

    virtual void Unmap() override;

    virtual void Resize(UINT32 width, UINT32 height) override;

    virtual void *ReadPixel(UINT32 handle, int x, int y, Format format, int width, int height) override;

    virtual void ClearAttachment(UINT32 attachmentIndex, int value) override;

private:
    void *handle{ nullptr };
};

}
}
