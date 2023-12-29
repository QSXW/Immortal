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

RenderTarget::RenderTarget() :
    Handle{ 0 }
{

}

RenderTarget::RenderTarget(uint32_t width, uint32_t height, const Format *pColorAttachmentFormats, uint32_t colorAttachmentCount, Format depthAttachmentFormat) :
    Handle{}
{
	glCreateFramebuffers(1, &handle);
	glBindFramebuffer(GL_FRAMEBUFFER, handle);
	Construct(width, height, pColorAttachmentFormats, colorAttachmentCount, depthAttachmentFormat);
}

RenderTarget::~RenderTarget()
{
	Release();
}

void RenderTarget::Resize(uint32_t _width, uint32_t _height)
{
	width  = _width;
	height = _height;
	
    for (auto &colorAttachments : attachments.color)
    {
		glTextureStorage2D(colorAttachments, 0, colorAttachments.GetFormat(), width, height);
    }
    if (attachments.depth)
    {
		glTextureStorage2D(attachments.depth, 0, attachments.depth.GetFormat(), width, height);
    }
}

SuperTexture *RenderTarget::GetColorAttachment(uint32_t index)
{
	return &attachments.color[index];
}

SuperTexture *RenderTarget::GetDepthAttachment()
{
	return &attachments.depth;
}

void RenderTarget::Release()
{
	attachments.color.clear();
	attachments.depth = {};
    glDeleteFramebuffers(1, &handle);
	handle = Handle::Invalid;
}

void RenderTarget::Construct(uint32_t width, uint32_t height, const Format *pColorAttachmentFormats, uint32_t colorAttachmentCount, Format depthAttachmentFormat)
{
	ThrowIf(colorAttachmentCount >= 4, "OpenGL backend only support 4 color attchments");

    if ((handle))
    {
		Release();
    }

    bool multisampled = false;
	auto target = SelectTextureTarget(multisampled);
    
    for (uint32_t i = 0; i < colorAttachmentCount; i++)
    {
		attachments.color.emplace_back(std::move(Texture{ pColorAttachmentFormats[i], width, height, 1, 1, TextureType::ColorAttachment }));
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, target, attachments.color[i], 0);
    }

    if (depthAttachmentFormat != Format::None)
    {
		attachments.depth = { depthAttachmentFormat, width, height, 1, 1, TextureType::DepthStencilAttachment };
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, target, attachments.depth, 0);
    }

    if (!attachments.color.empty())
    {
        GLenum buffers[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
		glDrawBuffers((GLsizei) attachments.color.size(), buffers);
    }
    else
    {
        glDrawBuffer(GL_NONE);
    }

    ThrowIf(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete!");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

}
}
