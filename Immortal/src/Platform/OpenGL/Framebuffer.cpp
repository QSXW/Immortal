#include "impch.h"
#include "Framebuffer.h"

#include "Common.h"
#include "Texture.h"

namespace Immortal
{
namespace OpenGL
{

Framebuffer::Framebuffer(const Framebuffer::Super::Description &descrition) :
    handle{ 0 },
    depthAttachment{ 0 },
    desc{ descrition }
{
    for (auto spec : desc.Attachments)
    {
        if (!Texture::IsDepthFormat(spec.Format))
        {
            colorAttachmentDescriptions.emplace_back(spec);
        }
        else
        {
            depthAttachmentDescription = spec;
        }
    }

    Update();
}

Framebuffer::~Framebuffer()
{
    Clear();
}

void Framebuffer::Clear()
{
    glDeleteFramebuffers(1, &handle);
    glDeleteTextures((GLsizei)colorAttachments.size(), colorAttachments.data());
    glDeleteTextures(1, &depthAttachment);

    colorAttachments.clear();
    depthAttachment = 0;
}

void Framebuffer::AttachColorTexture(uint32_t id, int samples, Texture::DataType type, uint32_t width, uint32_t height, int index)
{
    bool multisampled = samples > 1;
    if (multisampled)
    {
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, type.InternalFromat, width, height, GL_FALSE);
    }
    else
    {
        glTexImage2D(GL_TEXTURE_2D, 0, type.InternalFromat, width, height, 0, type.DataFormat, type.BinaryType, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, type.Filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, type.Filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, type.Wrap);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, type.Wrap);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, type.Wrap);
    }

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, TextureTarget(multisampled), id, 0);
}

void Framebuffer::AttachDepthTexture(uint32_t id, int samples, Texture::DataType type, uint32_t attachmentType, uint32_t width, uint32_t height)
{
    bool multisampled = samples > 1;
    if (multisampled)
    {
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, type.InternalFromat, width, height, GL_FALSE);
    }
    else
    {
        glTexStorage2D(GL_TEXTURE_2D, 1, type.InternalFromat, width, height);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, type.Filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, type.Filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, type.Wrap);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, type.Wrap);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, type.Wrap);
    }

    glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, TextureTarget(multisampled), id, 0);
}

void Framebuffer::Update()
{
    if ((handle))
    {
        Clear();
    }

    // @Create Framebuffer
    glCreateFramebuffers(1, &handle);
    glBindFramebuffer(GL_FRAMEBUFFER, handle);

    bool multisample = desc.Samples > 1;

    // Attachments
    if (colorAttachmentDescriptions.size())
    {
        colorAttachments.resize(colorAttachmentDescriptions.size());
        CreateTexture(multisample, colorAttachments.data(), (uint32_t)colorAttachments.size());

        for (size_t i = 0; i < colorAttachments.size(); i++)
        {
            BindTexture(multisample, colorAttachments[i]);
            Texture::DataType type = NativeTypeToOpenGl(colorAttachmentDescriptions[i].Format, colorAttachmentDescriptions[i].Wrap, colorAttachmentDescriptions[i].Filter);
            AttachColorTexture(colorAttachments[i], desc.Samples, type, desc.Width, desc.Height, (int)i);
        }
    }

    if (depthAttachmentDescription.Format != Format::None)
    {
        CreateTexture(multisample, &depthAttachment, 1);
        BindTexture(multisample, depthAttachment);
        Texture::DataType type = NativeTypeToOpenGl(depthAttachmentDescription.Format, depthAttachmentDescription.Wrap, depthAttachmentDescription.Filter);
        AttachDepthTexture(depthAttachment, desc.Samples, type, GL_DEPTH_STENCIL_ATTACHMENT, desc.Width, desc.Height);
    }

    if (colorAttachments.size() > 1)
    {
        if (colorAttachments.size() <= 4)
        {
            throw Exception{ "Only Support Four Color Attchments" };
        }
        GLenum buffers[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
        glDrawBuffers((GLsizei)colorAttachments.size(), buffers);
    }
    else if (colorAttachments.empty())
    {
        glDrawBuffer(GL_NONE);
    }

    SLASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete!");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::Map()
{
    glBindFramebuffer(GL_FRAMEBUFFER, handle);
    glViewport(0, 0, desc.Width, desc.Height);
}

void Framebuffer::Unmap()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::Resize(uint32_t width, uint32_t height)
{
    desc.Width = width;
    desc.Height = height;
    this->Update();
}

void *Framebuffer::ReadPixel(uint32_t index, int x, int y, Format format, int width, int height)
{
    this->Map();
    if (index >= colorAttachments.size())
    {
        throw Exception{ SError::OutOfBound };
    }
    glReadBuffer(GL_COLOR_ATTACHMENT0 + index);

    int pixel = -1;
    if (format == Format::RedInterger)
    {
        glReadPixels(x, y, width, height, GL_RED_INTEGER, GL_INT, &pixel);
    }
    else if (format == Format::RGBA8)
    {
        glReadPixels(x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, &pixel);
    }

    return reinterpret_cast<void *>((size_t)pixel);;
}

void Framebuffer::ClearAttachment(uint32_t index, int value)
{
    if (index >= colorAttachments.size())
    {
        throw Exception{ SError::OutOfBound };
    }

    auto type = NativeTypeToOpenGl(colorAttachmentDescriptions[index].Format);
    glClearTexImage(colorAttachments[index], 0, type.DataFormat, type.BinaryType, &value);
}

Attachment Framebuffer::ColorAttachment(size_t index)
{
    if (index >= colorAttachments.size())
    {
        throw Exception{ SError::OutOfBound };
    }
    return Attachment{ colorAttachments[index] };
}

Attachment Framebuffer::DepthAttachment()
{
    return Attachment{ depthAttachment };
}

}
}
