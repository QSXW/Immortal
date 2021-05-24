#pragma once

#include "ImmortalCore.h"
#include "Immortal/Render/Framebuffer.h"

namespace Immortal {

	class IMMORTAL_API D3D12Framebuffer : public Framebuffer
	{
	public:
		D3D12Framebuffer(const Framebuffer::Specification &spec)
		{

		}

		~D3D12Framebuffer() = default;

		virtual void Bind() override;
		virtual void Unbind() override;

		virtual void Resize(uint32_t width, uint32_t height) override;
		virtual void* ReadPixel(uint32_t attachmentIndex, int x, int y, Texture::Format format, int width = 1, int height = 1) override;

		virtual void ClearAttachment(uint32_t attachmentIndex, int value) override;

		virtual uint32_t ColorAttachmentRendererID(uint32_t index = 0) const override;
		virtual uint32_t DepthAttachmentRendererID(uint32_t index = 0) const override;
		virtual const Specification& GetSpecification() const override;
	};

}

