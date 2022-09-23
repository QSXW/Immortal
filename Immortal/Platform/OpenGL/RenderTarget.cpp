#include "RenderTarget.h"

#include "Common.h"
#include "Texture.h"

namespace Immortal
{
namespace OpenGL
{

static inline GLenum SelectTextureTarget(bool multiSampled)
{
	return multiSampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
}

RenderTarget::RenderTarget(const RenderTarget::Super::Description &descrition) :
    handle{ 0 },
    depthAttachment{ 0 },
    Super{ descrition }
{
    for (auto &attachment : desc.Attachments)
    {
        if (!attachment.IsDepth())
        {
            colorAttachmentDescriptions.emplace_back(attachment);
        }
        else
        {
            depthAttachmentDescription = attachment;
        }
    }

    Update();
}

RenderTarget::~RenderTarget()
{
    Clear();
}

void RenderTarget::Clear()
{
    glDeleteFramebuffers(1, &handle);
    glDeleteTextures((GLsizei)colorAttachments.size(), colorAttachments.data());
    glDeleteTextures(1, &depthAttachment);

    colorAttachments.clear();
    depthAttachment = 0;
}

void RenderTarget::AttachColorTexture(uint32_t id, int samples, Texture::DataType type, uint32_t width, uint32_t height, int index)
{
    bool multisampled = samples > 1;
    if (multisampled)
    {
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, type.SizedFormat, width, height, GL_FALSE);
    }
    else
    {
        glTexImage2D(GL_TEXTURE_2D, 0, type.SizedFormat, width, height, 0, type.BaseFormat, type.BinaryType, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, type.Filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, type.Filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, type.Wrap);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, type.Wrap);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, type.Wrap);
    }

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, SelectTextureTarget(multisampled), id, 0);
}

void RenderTarget::AttachDepthTexture(uint32_t id, int samples, Texture::DataType type, uint32_t attachmentType, uint32_t width, uint32_t height)
{
    bool multisampled = samples > 1;
    if (multisampled)
    {
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, type.SizedFormat, width, height, GL_FALSE);
    }
    else
    {
        glTexStorage2D(GL_TEXTURE_2D, 1, type.SizedFormat, width, height);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, type.Filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, type.Filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, type.Wrap);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, type.Wrap);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, type.Wrap);
    }

    glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, SelectTextureTarget(multisampled), id, 0);
}

void RenderTarget::Update()
{
    if ((handle))
    {
        Clear();
    }

	auto target = SelectTextureTarget(desc.Samples > 1);

    glCreateFramebuffers(1, &handle);
    glBindFramebuffer(GL_FRAMEBUFFER, handle);


    // Attachments
    if (colorAttachmentDescriptions.size())
    {
        colorAttachments.resize(colorAttachmentDescriptions.size());
		glCreateTextures(target, (uint32_t)colorAttachments.size(), colorAttachments.data());

        for (size_t i = 0; i < colorAttachments.size(); i++)
        {
			glBindTexture(target, colorAttachments[i]);
			Texture::DataType type = NativeTypeToOpenGl(colorAttachmentDescriptions[i]);
            AttachColorTexture(colorAttachments[i], desc.Samples, type, desc.Width, desc.Height, (int)i);
        }
    }

    if (depthAttachmentDescription.format != Format::None)
    {
		glCreateTextures(target, 1, &depthAttachment);
		glBindTexture(target, depthAttachment);
        Texture::DataType type = NativeTypeToOpenGl(depthAttachmentDescription);
        AttachDepthTexture(depthAttachment, desc.Samples, type, GL_DEPTH_STENCIL_ATTACHMENT, desc.Width, desc.Height);
    }

    if (colorAttachments.size() > 1)
    {
        ThrowIf(colorAttachments.size() >= 4, "Only Support Four Color Attchments");
        GLenum buffers[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
        glDrawBuffers((GLsizei)colorAttachments.size(), buffers);
    }
    else if (colorAttachments.empty())
    {
        glDrawBuffer(GL_NONE);
    }

    ThrowIf(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete!");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderTarget::Activate()
{
    glBindFramebuffer(GL_FRAMEBUFFER, handle);
    glViewport(0, 0, desc.Width, desc.Height);

    glClearColor(clearValues[0].r, clearValues[0].g, clearValues[0].b, clearValues[0].a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void RenderTarget::Deactivate()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderTarget::Resize(uint32_t width, uint32_t height)
{
    desc.Width = width;
    desc.Height = height;
    Update();
}

void *RenderTarget::ReadPixel(uint32_t index, int x, int y, Format format, int width, int height)
{
    Activate();
    if (index >= colorAttachments.size())
    {
        throw RuntimeException{ SError::OutOfBound };
    }
    glReadBuffer(GL_COLOR_ATTACHMENT0 + index);

    int pixel = -1;
    if (format == Format::R32)
    {
        glReadPixels(x, y, width, height, GL_RED_INTEGER, GL_INT, &pixel);
    }
    else if (format == Format::RGBA8)
    {
        glReadPixels(x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, &pixel);
    }

    return reinterpret_cast<void *>((size_t)pixel);;
}

Attachment RenderTarget::ColorAttachment(size_t index)
{
    if (index >= colorAttachments.size())
    {
        throw Exception{ SError::OutOfBound };
    }
    return Attachment{ colorAttachments[index] };
}

Attachment RenderTarget::DepthAttachment()
{
    return Attachment{ depthAttachment };
}

}
}
