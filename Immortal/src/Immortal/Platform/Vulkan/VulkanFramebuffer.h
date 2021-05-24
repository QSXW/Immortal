#pragma once

#include "Immortal/Render/Framebuffer.h"

#include "VulkanCommon.h"

namespace Immortal
{
	class VulkanFramebuffer : public Framebuffer
	{
	public:
		VulkanFramebuffer(const Framebuffer::Specification &spec);
		~VulkanFramebuffer();

		virtual void Bind() override;

		virtual void Unbind() override;

		virtual void Resize(UINT32 width, UINT32 height) override;

		virtual void* ReadPixel(UINT32 attachmentIndex, int x, int y, Texture::Format format, int width, int height) override;

		virtual void ClearAttachment(UINT32 attachmentIndex, int value) override;

		virtual UINT32 ColorAttachmentRendererID(UINT32 index) const override;

		virtual UINT32 DepthAttachmentRendererID(UINT32 index) const override;

		virtual const Specification& GetSpecification() const override;

	private:
		VkFramebuffer mHandle{ VK_NULL_HANDLE };
	};
}

