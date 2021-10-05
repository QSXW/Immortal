#pragma once

#include "ImmortalCore.h"

#include "Texture.h"

namespace Immortal
{
   
class Framebuffer
{
public:
    struct AttachmentDescription
    {
        AttachmentDescription() = default;

        AttachmentDescription(std::initializer_list<Texture::Description> attachments)
            : Attachments(attachments) {}

        std::vector<Texture::Description>::iterator begin()
        { 
            return Attachments.begin();
        }

        std::vector<Texture::Description>::iterator end()
        {
            return Attachments.end();
        }

        std::vector<Texture::Description>::const_iterator cbegin()
        {
            return Attachments.cbegin();
        }

        std::vector<Texture::Description>::const_iterator cend()
        {
            return Attachments.cend();
        }

        std::vector<Texture::Description> Attachments;
    };

    struct Description
    {
        Description() { }

        Description(UINT32 width, UINT32 height, AttachmentDescription attachments)
            : Width(width), Height(height), Attachments(attachments)
        {

        }

        UINT32 Width{ 0 };
        UINT32 Height{ 0 };
        AttachmentDescription Attachments{};
        UINT32 Samples = 1;
        UINT32 Layers{ 0 };
        bool SwapChainTarget = false;

        void *context{ nullptr };
    };

public:
    Framebuffer();

    Framebuffer(const Description &description) : 
        desc{ description }
    {
    
    }

    virtual ~Framebuffer() = default;

    virtual void Map() { }

    virtual void Unmap() { }
         
    virtual void Resize(UINT32 width, UINT32 height) { }

    virtual void *ReadPixel(UINT32 attachmentIndex, int x, int y, Texture::Format format, int width = 1, int height = 1)
    {
        return nullptr;
    }

    virtual void ClearAttachment(UINT32 attachmentIndex, int value) { }

    virtual UINT32 ColorAttachmentHandle(UINT32 index = 0) const
    {
        return 0;
    }

    virtual UINT32 DepthAttachmentHandle(UINT32 index = 0) const
    {
        return 0;
    }

    virtual const Description &Desc() const
    {
        return desc;
    }

    static std::shared_ptr<Framebuffer> Create(const Description &desc);

protected:
    Description desc{};
};

using SuperFramebuffer = Framebuffer;

}
