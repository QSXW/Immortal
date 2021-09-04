#include "impch.h"
#include "D3D12Framebuffer.h"

#include "D3D12Common.h"

namespace Immortal {
	void D3D12Framebuffer::Map()
	{

	}

	void D3D12Framebuffer::Unmap()
	{

	}

	void D3D12Framebuffer::Resize(uint32_t width, uint32_t height)
	{

	}

	void* D3D12Framebuffer::ReadPixel(uint32_t attachmentIndex, int x, int y, Texture::Format format, int width, int height)
	{
		return 0;
	}

	void D3D12Framebuffer::ClearAttachment(uint32_t attachmentIndex, int value)
	{

	}

	uint32_t D3D12Framebuffer::ColorAttachmentRendererID(uint32_t index) const
	{
		return uint32_t();
	}

	uint32_t D3D12Framebuffer::DepthAttachmentRendererID(uint32_t index) const
	{
		return uint32_t();
	}

	const Framebuffer::Specification & D3D12Framebuffer::GetSpecification() const
	{
		return *(new Framebuffer::Specification());
	}
}