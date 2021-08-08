#include "impch.h"
#include "OpenGLFramebuffer.h"

#include "OpenGLTexture.h"

#include <glad/glad.h>

namespace Immortal {

	OpenGLFramebuffer::OpenGLFramebuffer(const Framebuffer::Specification & spec)
		: mRendererID(0), mDepthAttachment(0), mSpecification(spec)
	{
		for (auto spec : mSpecification.Attachments)
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

	OpenGLFramebuffer::~OpenGLFramebuffer()
	{
		Clear();
	}

	void OpenGLFramebuffer::Clear()
	{
		glDeleteFramebuffers(1, &mRendererID);
		glDeleteTextures((GLsizei)mColorAttachments.size(), mColorAttachments.data());
		glDeleteTextures(1, &mDepthAttachment);

		mColorAttachments.clear();
		mDepthAttachment = 0;
	}

	void OpenGLFramebuffer::AttachColorTexture(uint32_t id, int samples, Texture::DataType type, uint32_t width, uint32_t height, int index)
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

	void OpenGLFramebuffer::AttachDepthTexture(uint32_t id, int samples, Texture::DataType type, uint32_t attachmentType, uint32_t width, uint32_t height)
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

	void OpenGLFramebuffer::Update()
	{
		if ((mRendererID))
		{
			Clear();
		}

		// @Create Framebuffer
		glCreateFramebuffers(1, &mRendererID);
		glBindFramebuffer(GL_FRAMEBUFFER, mRendererID);

		bool multisample = mSpecification.Samples > 1;

		// Attachments
		if (mColorAttachmentSpecifications.size())
		{
			mColorAttachments.resize(mColorAttachmentSpecifications.size());
			OpenGLTexture::CreateTexture(multisample, mColorAttachments.data(), (uint32_t)mColorAttachments.size());

			for (size_t i = 0; i < mColorAttachments.size(); i++)
			{
				OpenGLTexture::BindTexture(multisample, mColorAttachments[i]);
				Texture::DataType type = OpenGLTexture::NativeTypeToOpenGl(mColorAttachmentSpecifications[i].Format, mColorAttachmentSpecifications[i].Wrap, mColorAttachmentSpecifications[i].Filter);
				AttachColorTexture(mColorAttachments[i], mSpecification.Samples, type, mSpecification.Width, mSpecification.Height, (int)i);
			}
		}

		if (mDepthAttachmentSpecification.Format != Texture::Format::None)
		{
			OpenGLTexture::CreateTexture(multisample, &mDepthAttachment, 1);
			OpenGLTexture::BindTexture(multisample, mDepthAttachment);
			Texture::DataType type = OpenGLTexture::NativeTypeToOpenGl(mDepthAttachmentSpecification.Format, mDepthAttachmentSpecification.Wrap, mDepthAttachmentSpecification.Filter);
			AttachDepthTexture(mDepthAttachment, mSpecification.Samples, type, GL_DEPTH_STENCIL_ATTACHMENT, mSpecification.Width, mSpecification.Height);
		}

		if (mColorAttachments.size() > 1)
		{
			IM_CORE_ASSERT(mColorAttachments.size() <= 4, "Only Support Four Color Attchments");
			GLenum buffers[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
			glDrawBuffers((GLsizei)mColorAttachments.size(), buffers);
		}
		else if (mColorAttachments.empty())
		{
			// Only depth-pass
			glDrawBuffer(GL_NONE);
		}

		IM_CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete!");
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLFramebuffer::Bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, mRendererID);
		glViewport(0, 0, mSpecification.Width, mSpecification.Height);
	}

	void OpenGLFramebuffer::Unbind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLFramebuffer::Resize(uint32_t width, uint32_t height)
	{
		mSpecification.Width = width;
		mSpecification.Height = height;
		this->Update();
	}

	void* OpenGLFramebuffer::ReadPixel(uint32_t attachmentIndex, int x, int y, Texture::Format format, int width, int height)
	{
		this->Bind();
		IM_CORE_ASSERT(attachmentIndex < mColorAttachments.size(), "The attachmentIndex out of bound.");
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

		return reinterpret_cast<void*>((int)pixel);;
	}

	void OpenGLFramebuffer::ClearAttachment(uint32_t index, int value)
	{
		IM_CORE_ASSERT(index < mColorAttachments.size(), "OpenGLFramebuffer::ClearAttachment: index out of bound");

		auto type = OpenGLTexture::NativeTypeToOpenGl(mColorAttachmentSpecifications[index].Format);
		glClearTexImage(mColorAttachments[index], 0, type.DataFormat, type.BinaryType, &value);
	}

	uint32_t OpenGLFramebuffer::ColorAttachmentRendererID(uint32_t index) const 
	{
		IM_CORE_ASSERT(index < mColorAttachments.size(), "OpenGLFramebuffer::ColorAttachmentRendererID: index out of bound");
		return mColorAttachments[index];
	}

	uint32_t OpenGLFramebuffer::DepthAttachmentRendererID(uint32_t index) const
	{
		return mDepthAttachment;
	}

}