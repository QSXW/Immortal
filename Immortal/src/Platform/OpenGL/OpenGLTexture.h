#pragma once

#include "ImmortalCore.h"

#include "Render/Texture.h"

#include <glad/glad.h>

namespace Immortal {

	namespace OpenGLTexture
	{
		static UINT32 Create(GLenum target, const uint32_t width, const uint32_t height, Texture::DataType &spec, int levels = 0);

		static inline GLenum TextureTarget(bool multisampled)
		{
			return multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
		}

		static inline void CreateTexture(bool multisampled, uint32_t *outTextureID, uint32_t count)
		{
			glCreateTextures(TextureTarget(multisampled), count, outTextureID);
		}

		static inline void BindTexture(bool multisampled, uint32_t id)
		{
			glBindTexture(TextureTarget(multisampled), id);
		}

		static Texture::DataType NativeTypeToOpenGl(Texture::Format format, Texture::Wrap wrap = Texture::Wrap::Clamp, Texture::Filter filter = Texture::Filter::Linear)
		{
			Texture::DataType data;

			data.Wrap = wrap == Texture::Wrap::Clamp ? GL_CLAMP_TO_EDGE : GL_REPEAT;
			data.Filter = filter == Texture::Filter::Linear ? GL_LINEAR : GL_NEAREST;
			switch (format)
			{
			case Texture::Format::RedInterger:
				data.InternalFromat = GL_R32I;
				data.DataFormat = GL_RED_INTEGER;
				data.BinaryType = GL_INT;
				break;

			case Texture::Format::RGBA:
			case Texture::Format::RGBA8:
				data.InternalFromat = GL_RGBA8;
				data.DataFormat = GL_RGBA;
				data.BinaryType = GL_UNSIGNED_BYTE;
				break;

			case Texture::Format::RGB:
			case Texture::Format::RGB8:
				data.InternalFromat = GL_RGB8;
				data.DataFormat = GL_RGB;
				data.BinaryType = GL_UNSIGNED_BYTE;
				break;

			case Texture::Format::RG16F:
				data.InternalFromat = GL_RG16F;
				data.DataFormat = GL_RG;
				data.BinaryType = GL_FLOAT;
				break;

			case Texture::Format::RGBA16F:
				data.InternalFromat = GL_RGBA16F;
				data.DataFormat = GL_RGBA;
				data.BinaryType = GL_FLOAT;
				break;

			case Texture::Format::RGB16F:
				data.InternalFromat = GL_RGB16F;
				data.DataFormat = GL_RGB;
				data.BinaryType = GL_FLOAT;
				break;

			case Texture::Format::RGBA32F:
				data.InternalFromat = GL_RGBA32F;
				data.DataFormat = GL_RGBA;
				data.BinaryType = GL_FLOAT;
				break;

			case Texture::Format::RGB32F:
				data.InternalFromat = GL_RGB32F;
				data.DataFormat = GL_RGB;
				data.BinaryType = GL_FLOAT;
				break;

			case Texture::Format::Depth24Stencil8:
				data.InternalFromat = GL_DEPTH24_STENCIL8;
				break;

			default:
				SLASSERT(false && "Invalid Texture Format.");
				break;
			}

			return data;
		}

		static Texture::DataType NativeTypeToOpenGl(Texture::Description &spec)
		{
			return NativeTypeToOpenGl(spec.Format, spec.Wrap, spec.Filter);
		}
	}

	class OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D(uint32_t width, uint32_t height);
		OpenGLTexture2D(const std::string& path, bool flip = true);
		OpenGLTexture2D(const uint32_t width, const uint32_t height, Texture::Description &spec, int levels);
		OpenGLTexture2D(const std::string & path, bool flip, Texture::Wrap wrap, Texture::Filter filter);
		OpenGLTexture2D(const std::string & path, Texture::Wrap wrap, Texture::Filter filter);
		OpenGLTexture2D(UINT32 width, UINT32 height, const void * data, Texture::Description & spec, INT32 level = 0);

		virtual ~OpenGLTexture2D();

		virtual uint32_t Width() const override
		{ 
			return mWidth;
		}

		virtual uint32_t Height() const override
		{ 
			return mHeight;
		}

		virtual uint32_t Handle() const override
		{ 
			return mTexture; 
		}

		virtual void SetData(void* data, uint32_t size) override;

		virtual void Map(uint32_t slot = 0) const override;
		virtual void BindImageTexture(bool layered = false) override;

		virtual uint32_t MipLevelCount() const override;

		virtual bool operator==(const Texture& other) const override
		{
			return mTexture == ((OpenGLTexture2D&)other).mTexture;
		}
		virtual float Ratio() const override
		{
			return mRatio;
		}

		virtual const char *Path() const override { return mPath.c_str(); }

	private:
		std::string mPath;
		uint32_t mWidth;
		uint32_t mHeight;
		uint32_t mTexture;
		int32_t mLevel;
		float mRatio;

		Texture::DataType mType;
	};

	class OpenGLTextureCube : public TextureCube
	{
	public:
		OpenGLTextureCube(const uint32_t width, const uint32_t height, Texture::Description&, int levels=0);
		OpenGLTextureCube(const std::string& path);
		virtual ~OpenGLTextureCube();

		virtual void Map(uint32_t slot = 0) const;

		virtual uint32_t Width() const { return mWidth; }
		virtual uint32_t Height() const { return mHeight; }
		virtual float Ratio() const { return (float)mWidth / (float)mHeight; }

		// This function currently returns the expected number of mips based on image size,
		// not present mips in data
		virtual uint32_t MipLevelCount() const override;
		virtual void SetData(void *data, uint32_t size) override;

		virtual const char *Path() const override { return mFilePath.c_str(); }

		uint32_t Handle() const { return mTexture; }

		virtual bool operator==(const Texture& other) const override
		{
			return mTexture == ((OpenGLTextureCube&)other).mTexture;
		}

		void Create(const uint32_t width, const uint32_t height, Texture::Description &spec, int levels = 0);
		virtual void BindImageTexture(bool layered = false) override;

	private:
		uint32_t mTexture = 0;
		uint32_t mWidth = 0;
		uint32_t mHeight = 0;
		int32_t mLevel = 0;

		Texture::Description mSpecification;
		Texture::DataType mType;
		std::string mFilePath;
	};

}

