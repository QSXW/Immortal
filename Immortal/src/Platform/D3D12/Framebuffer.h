#pragma once

#include "Render/Framebuffer.h"

#include "D3D12Common.h"

namespace Immortal
{
namespace D3D12
{
    class Framebuffer : public Immortal::Framebuffer
    {
    public:
        using Super = Immortal::Framebuffer;

    public:
        Framebuffer(const Super::Specification &spec);
        ~Framebuffer();

        virtual void Map() override;

        virtual void Unmap() override;

        virtual void Resize(UINT32 width, UINT32 height) override;

        virtual void *ReadPixel(UINT32 handle, int x, int y, Texture::Format format, int width, int height) override;

        virtual void ClearAttachment(UINT32 attachmentIndex, int value) override;

        virtual UINT32 ColorAttachmentRendererID(UINT32 index) const override;

        virtual UINT32 DepthAttachmentRendererID(UINT32 index) const override;

        virtual const Super::Specification &GetSpecification() const override;

    private:
        void *handle{ nullptr };
    };
}
}
