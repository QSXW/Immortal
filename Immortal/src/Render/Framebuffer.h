#pragma once

#include "ImmortalCore.h"

#include "Texture.h"

namespace Immortal
{
   
struct Attachment
{
public:
    enum class Type
    {
        Color,
        Depth
    };

    struct Description
    {
        Description() = default;

        Description(std::initializer_list<Texture::Description> attachments)
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

public:
    Attachment() :
        handle{ nullptr }
    {
    
    }

    Attachment(uint32_t handle) :
        handle{ (void*)(uint64_t)(handle) }
    {
        
    }

    operator uint64_t()
    {
        return rcast<uint64_t>(handle);
    }

    void *handle;
};

class Framebuffer
{
public:
    struct Description
    {
        Description() { }

        Description(Vector2 viewport, Attachment::Description attachments) :
            Width{ ncast<UINT32>(viewport.x) },
            Height{ ncast<UINT32>(viewport.y) },
            Attachments(attachments)
        {

        }

        Description(UINT32 width, UINT32 height, Attachment::Description attachments)
            : Width(width), Height(height), Attachments(attachments)
        {

        }

        UINT32 Width{ 0 };

        UINT32 Height{ 0 };

        UINT32 Samples = 1;

        UINT32 Layers{ 0 };

        Attachment::Description Attachments{};

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

    virtual void Resize(Vector2 size)
    {
        Resize(size.x, size.y);
    }

    virtual void *ReadPixel(UINT32 attachmentIndex, int x, int y, Format format, int width = 1, int height = 1)
    {
        return nullptr;
    }

    virtual void ClearAttachment(UINT32 attachmentIndex, int value) { }

    template <Attachment::Type T, class ... Args>
    Attachment &Get(Args&& ... args)
    {
        if constexpr (T == Attachment::Type::Color)
        {
            return ColorAttachment(std::forward<Args>(args)...);
        }
        if constexpr (T == Attachment::Type::Depth)
        {
            return DepthAttachment(std::forward<Args>(args)...);
        }
    }

    virtual Attachment ColorAttachment(size_t index = 0)
    {
        return Attachment{};
    }

    virtual Attachment DepthAttachment()
    {
        return Attachment{};
    }

    const Description &Desc() const
    {
        return desc;
    }

protected:
    Description desc{};
};

using SuperFramebuffer = Framebuffer;

}
