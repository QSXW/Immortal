#include "impch.h"
#include "Framebuffer.h"

#include "OpenGLTexture.h"

#include <glad/glad.h>

namespace Immortal
{
namespace OpenGL
{
Framebuffer::Framebuffer(const Framebuffer::Super::Description &descrition) :
    handle{ 0 },
    mDepthAttachment{ 0 },
    desc{ descrition }
{
    for (auto spec : desc.Attachments)
    {
        if (!Texture::IsDepthFormat(spec.Format))
        {
            mColorAttachmentSpecifications.emplace_back(spec);
        }
        else
        {
            mDepthAttachmentSpecification = spec;
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
    glDeleteTextures((GLsizei)mColorAttachments.size(), mColorAttachments.data());
    glDeleteTextures(1, &mDepthAttachment);

    mColorAttachments.clear();
    mDepthAttachment = 0;
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

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, OpenGLTexture::TextureTarget(multisampled), id, 0);
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

    glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, OpenGLTexture::TextureTarget(multisampled), id, 0);
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
    if (mColorAttachmentSpecifications.size())
    {
        mColorAttachments.resize(mColorAttachmentSpecifications.size());
        OpenGLTexture::CreateTexture(multisample, mColorAttachments.data(), (uint32_t)mColorAttachments.size());

        for (size_t i = 0; i < mColorAttachments.size(); i++)
        {
            OpenGLTexture::BindTexture(multisample, mColorAttachments[i]);
            Texture::DataType type = OpenGLTexture::NativeTypeToOpenGl(mColorAttachmentSpecifications[i].Format, mColorAttachmentSpecifications[i].Wrap, mColorAttachmentSpecifications[i].Filter);
            AttachColorTexture(mColorAttachments[i], desc.Samples, type, desc.Width, desc.Height, (int)i);
        }
    }

    if (mDepthAttachmentSpecification.Format != Texture::Format::None)
    {
        OpenGLTexture::CreateTexture(multisample, &mDepthAttachment, 1);
        OpenGLTexture::BindTexture(multisample, mDepthAttachment);
        Texture::DataType type = OpenGLTexture::NativeTypeToOpenGl(mDepthAttachmentSpecification.Format, mDepthAttachmentSpecification.Wrap, mDepthAttachmentSpecification.Filter);
        AttachDepthTexture(mDepthAttachment, desc.Samples, type, GL_DEPTH_STENCIL_ATTACHMENT, desc.Width, desc.Height);
    }

    if (mColorAttachments.size() > 1)
    {
        SLASSERT(mColorAttachments.size() <= 4 && "Only Support Four Color Attchments");
        GLenum buffers[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
        glDrawBuffers((GLsizei)mColorAttachments.size(), buffers);
    }
    else if (mColorAttachments.empty())
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

void *Framebuffer::ReadPixel(uint32_t attachmentIndex, int x, int y, Texture::Format format, int width, int height)
{
    this->Map();
    SLASSERT(attachmentIndex < mColorAttachments.size() && "The attachmentIndex out of bound.");
    glReadBuffer(GL_COLOR_ATTACHMENT0 + attachmentIndex);

    int pixel = -1;
    if (format == Texture::Format::RedInterger)
    {
        glReadPixels(x, y, width, height, GL_RED_INTEGER, GL_INT, &pixel);

    }
    else if (format == Texture::Format::RGBA8)
    {
        glReadPixels(x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, &pixel);
    }

    return reinterpret_cast<void *>((size_t)pixel);;
}

void Framebuffer::ClearAttachment(uint32_t index, int value)
{
    SLASSERT(index < mColorAttachments.size() && "Framebuffer::ClearAttachment: index out of bound");

    auto type = OpenGLTexture::NativeTypeToOpenGl(mColorAttachmentSpecifications[index].Format);
    glClearTexImage(mColorAttachments[index], 0, type.DataFormat, type.BinaryType, &value);
}

uint32_t Framebuffer::ColorAttachmentRendererID(uint32_t index) const
{
    SLASSERT(index < mColorAttachments.size() && "Framebuffer::ColorAttachmentRendererID: index out of bound");
    return mColorAttachments[index];
}

uint32_t Framebuffer::DepthAttachmentRendererID(uint32_t index) const
{
    return mDepthAttachment;
}

}
}
