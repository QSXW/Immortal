#pragma once

#include "ImmortalCore.h"

#include "Texture.h"

namespace Immortal {
	
	/**
	 * @brief A Framebuffer, or render pass, represents a collection of attachments,
	 * subpasses, and dependencies between the subpasses, and describes how the attach-
	 * ments are used over the course of the subpasses. The use of a render pass in
	 * a command buffer is a render pass instance. A render pass is just a framebuffer
	 * minimumly.
	 *
	 * <a href="https://www.youtube.com/watch?v=x2SGVjlVGhE">Render Passes</a>
	 * 
	 */
	class Framebuffer
	{
	public:

		struct AttachmentDiscription
		{
			AttachmentDiscription() = default;
			AttachmentDiscription(std::initializer_list<Texture::Description> attachments)
				: Attachments(attachments) {}

			NODISCARD std::vector<Texture::Description>::iterator begin() NOEXCEPT { return Attachments.begin(); }
			NODISCARD std::vector<Texture::Description>::iterator end() NOEXCEPT { return Attachments.end(); }
			NODISCARD std::vector<Texture::Description>::const_iterator cbegin() NOEXCEPT { return Attachments.cbegin(); }
			NODISCARD std::vector<Texture::Description>::const_iterator cend() NOEXCEPT { return Attachments.cend(); }
			std::vector<Texture::Description> Attachments;
		};

		struct Specification
		{
			Specification() { }
			Specification(UINT32 width, UINT32 height, AttachmentDiscription attachments)
				: Width(width), Height(height), Attachments(attachments)
			{

			}

			UINT32 Width  = 0;
			UINT32 Height = 0;
			AttachmentDiscription Attachments{};
			UINT32 Samples = 1;
			UINT32 Layers = 0;
			/** 
			 * @brief Equals to glBindFramebuffer(0). Allow to create a framebuffer that doesn't
			 * really exist.. if SwapChainTarget is true, which means it's not an off-screen buffer
			 * and just render the render pass to the screen SwapChainTarget = screen buffer
			 * (i.e. no framebuffer) For D3D12 API.
			 * 
			 */
			bool SwapChainTarget = false;
		};

	public:
		virtual ~Framebuffer() = default;

		virtual void Map() = 0;
		virtual void UnMap() = 0;
		 
		virtual void Resize(UINT32 width, UINT32 height) = 0;
		virtual void* ReadPixel(UINT32 attachmentIndex, int x, int y, Texture::Format format, int width = 1, int height = 1) = 0;

		virtual void ClearAttachment(UINT32 attachmentIndex, int value) = 0;

		virtual UINT32 ColorAttachmentRendererID(UINT32 index = 0) const = 0;
		virtual UINT32 DepthAttachmentRendererID(UINT32 index = 0) const = 0;
		virtual const Specification& GetSpecification() const = 0;

		static Ref<Framebuffer> Create(const Specification& spec);
	};


}

