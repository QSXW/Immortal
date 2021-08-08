#include "impch.h"
#include "VulkanFramebuffer.h"

namespace Immortal
{
	VulkanFramebuffer::VulkanFramebuffer(const Framebuffer::Specification & spec)
	{

	}

	VulkanFramebuffer::~VulkanFramebuffer()
	{

	}

	void VulkanFramebuffer::Bind()
	{
	}

	void VulkanFramebuffer::Unbind()
	{
	}

	void VulkanFramebuffer::Resize(UINT32 width, UINT32 height)
	{
	}

	void * VulkanFramebuffer::ReadPixel(UINT32 attachmentIndex, int x, int y, Texture::Format format, int width, int height)
	{
		return nullptr;
	}

	void VulkanFramebuffer::ClearAttachment(UINT32 attachmentIndex, int value)
	{
	}

	UINT32 VulkanFramebuffer::ColorAttachmentRendererID(UINT32 index) const
	{
		return UINT32();
	}

	UINT32 VulkanFramebuffer::DepthAttachmentRendererID(UINT32 index) const
	{
		return UINT32();
	}

	const Framebuffer::Specification &VulkanFramebuffer::GetSpecification() const
	{
		return Framebuffer::Specification();
	}

}