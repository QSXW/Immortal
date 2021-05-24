#include "impch.h"
#include "OpenGLTexture.h"

#include "Immortal/Render/Frame.h"

#include "OpenGLShader.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Immortal {

	UINT32 __forceinline OpenGLTexture::Create(GLenum target, const uint32_t width, const uint32_t height, Texture::DataType & type, int level)
	{
		UINT32 texture;

		glCreateTextures(target, 1, &texture);
		glTextureStorage2D(texture, level, type.InternalFromat, width, height);
		glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, texture > 1 ? GL_LINEAR_MIPMAP_LINEAR : type.Filter);
		glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, type.Filter);

		glTextureParameteri(texture, GL_TEXTURE_WRAP_S, type.Wrap);
		glTextureParameteri(texture, GL_TEXTURE_WRAP_T, type.Wrap);
		glTextureParameteri(texture, GL_TEXTURE_WRAP_R, type.Wrap);

		return texture;
	}

	OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height)
		: mWidth(width), 
		  mHeight(height), 
		  mTexture(0),
		  mRatio((float)width / (float)height)
	{
		mType.InternalFromat = GL_RGBA8;
		mType.DataFormat     = GL_RGBA;
		mType.BinaryType     = GL_UNSIGNED_BYTE;

		glCreateTextures(GL_TEXTURE_2D, 1, &mTexture);
		glTextureStorage2D(mTexture, 1, mType.InternalFromat, mWidth, mHeight);

		glTextureParameteri(mTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(mTexture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTextureParameteri(mTexture, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(mTexture, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	OpenGLTexture2D::OpenGLTexture2D(const std::string & path, bool flip, Texture::Wrap wrap, Texture::Filter filter)
	{
		IM_CORE_TRACE("{0} Loading: {1}", __func__, path);
		Frame frame = Frame(path, flip);

		mWidth = frame.Width();
		mHeight = frame.Height();
		mRatio = (float)mWidth / (float)mHeight;

		mLevel = Texture::CalculateMipmapLevels(mWidth, mHeight);
		mType = OpenGLTexture::NativeTypeToOpenGl(frame.Type().Format, wrap, filter);
		mTexture = OpenGLTexture::Create(GL_TEXTURE_2D, mWidth, mHeight, mType, mLevel);

		glTextureSubImage2D(mTexture, 0, 0, 0, mWidth, mHeight, mType.DataFormat, mType.BinaryType, frame.Data());

		if (mLevel > 0)
		{
			glGenerateTextureMipmap(mTexture);
		}
	}

	OpenGLTexture2D::OpenGLTexture2D(const std::string & path, bool flip)
		: mPath(path), mTexture(0)
	{
		IM_CORE_TRACE("{0} Loading: {1}", __func__, path);
		Frame frame = Frame(path, flip);

		mWidth  = frame.Width();
		mHeight = frame.Height();
		mRatio  = (float)mWidth / (float)mHeight;

		mType = OpenGLTexture::NativeTypeToOpenGl(frame.Type());

		glCreateTextures(GL_TEXTURE_2D, 1, &mTexture);
		glTextureStorage2D(mTexture, 1, mType.InternalFromat, mWidth, mHeight);
		glTextureSubImage2D(mTexture, 0, 0, 0, mWidth, mHeight, mType.DataFormat, mType.BinaryType, frame.Data());

		glTextureParameteri(mTexture, GL_TEXTURE_MIN_FILTER, mType.Filter);
		glTextureParameteri(mTexture, GL_TEXTURE_MAG_FILTER, mType.Filter);

		glTextureParameteri(mTexture, GL_TEXTURE_WRAP_S, mType.Wrap);
		glTextureParameteri(mTexture, GL_TEXTURE_WRAP_T, mType.Wrap);

		mLevel = Texture::CalculateMipmapLevels(mWidth, mHeight);
		if (mLevel > 0)
		{
			glGenerateTextureMipmap(mTexture);
		}
		IM_CORE_TRACE("{0} Completed: {1}", __func__, path);
	}

	OpenGLTexture2D::OpenGLTexture2D(const std::string & path, Texture::Wrap wrap, Texture::Filter filter)
	{
		IM_CORE_TRACE("{0} Loading: {1}", __func__, path);
		Frame frame = Frame(path);

		mWidth = frame.Width();
		mHeight = frame.Height();
		mRatio = (float)mWidth / (float)mHeight;

		mType = OpenGLTexture::NativeTypeToOpenGl(frame.Type().Format, wrap, filter);

		glCreateTextures(GL_TEXTURE_2D, 1, &mTexture);
		glTextureStorage2D(mTexture, 1, mType.InternalFromat, mWidth, mHeight);
		glTextureSubImage2D(mTexture, 0, 0, 0, mWidth, mHeight, mType.DataFormat, mType.BinaryType, frame.Data());

		glTextureParameteri(mTexture, GL_TEXTURE_MIN_FILTER, mType.Filter);
		glTextureParameteri(mTexture, GL_TEXTURE_MAG_FILTER, mType.Filter);

		glTextureParameteri(mTexture, GL_TEXTURE_WRAP_S, mType.Wrap);
		glTextureParameteri(mTexture, GL_TEXTURE_WRAP_T, mType.Wrap);

		mLevel = Texture::CalculateMipmapLevels(mWidth, mHeight);
		if (mLevel > 0)
		{
			glGenerateTextureMipmap(mTexture);
		}
		IM_CORE_TRACE("{0} Completed: {1}", __func__, path);
	}

	OpenGLTexture2D::OpenGLTexture2D(const uint32_t width, const uint32_t height, Texture::Description & spec, int levels)
	{
		mWidth  = width;
		mHeight = height;
		mLevel = (levels > 0) ? levels : Texture::CalculateMipmapLevels(width, height);

		mType = OpenGLTexture::NativeTypeToOpenGl(spec);

		glCreateTextures(GL_TEXTURE_2D, 1, &mTexture);
		glTextureStorage2D(mTexture, mLevel, mType.InternalFromat, width, height);
		
		// glTextureParameterf(mTexture, GL_TEXTURE_MAX_ANISOTROPY, 16.0f);

		glTextureParameteri(mTexture, GL_TEXTURE_MIN_FILTER, mLevel > 1 ? GL_LINEAR_MIPMAP_LINEAR : mType.Filter);
		glTextureParameteri(mTexture, GL_TEXTURE_MAG_FILTER, mType.Filter);

		glTextureParameteri(mTexture, GL_TEXTURE_WRAP_S, mType.Wrap);
		glTextureParameteri(mTexture, GL_TEXTURE_WRAP_T, mType.Wrap);
	}


	OpenGLTexture2D::OpenGLTexture2D(UINT32 width, UINT32 height, const void *data, Texture::Description &spec, INT32 level)
		: mWidth(width), mHeight(height)
	{
		mLevel   = (level > 0) ? level : Texture::CalculateMipmapLevels(width, height);
		mType    = OpenGLTexture::NativeTypeToOpenGl(spec);
		mTexture = OpenGLTexture::Create(GL_TEXTURE_2D, mWidth, mHeight, mType, mLevel);

		glTextureSubImage2D(mTexture, 0, 0, 0, mWidth, mHeight, mType.DataFormat, mType.BinaryType, data);

		if (mLevel > 0)
		{
			glGenerateTextureMipmap(mTexture);
		}
	}

	OpenGLTexture2D::~OpenGLTexture2D()
	{
		glDeleteTextures(1, &mTexture);
	}

	void OpenGLTexture2D::SetData(void * data, uint32_t size)
	{
		uint32_t bpp = mType.DataFormat == GL_RGBA ? 4 : 3;
		IM_CORE_ASSERT(size == mWidth * mHeight * bpp, "Data must be entire texture!");

		// glTextureStorage2D(mTexture, 1, mInternalFormat, mWidth, mHeight);
		glTextureSubImage2D(mTexture, 0, 0, 0, mWidth, mHeight, mType.DataFormat, mType.BinaryType, data);
	}

	void OpenGLTexture2D::Bind(uint32_t slot) const
	{
		glBindTextureUnit(slot, mTexture);
	}

	uint32_t OpenGLTexture2D::MipLevelCount() const
	{
		return uint32_t();
	}

	void OpenGLTexture2D::BindImageTexture(bool layered)
	{
		glBindImageTexture(0, mTexture, 0, layered ? GL_TRUE : GL_FALSE, 0, GL_WRITE_ONLY, mType.InternalFromat);
	}

	OpenGLTextureCube::OpenGLTextureCube(const uint32_t width, const uint32_t height, Texture::Description &spec, int levels)
	{
		this->Create(width, height, spec, levels);
	}

	void OpenGLTextureCube::Create(const uint32_t width, const uint32_t height, Texture::Description &spec, int levels)
	{
		mWidth = width;
		mHeight = height;
		mLevel = (levels > 0) ? levels : Texture::CalculateMipmapLevels(width, height);
		mSpecification = spec;

		mType = OpenGLTexture::NativeTypeToOpenGl(spec);

		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &mTexture);
		glTextureStorage2D(mTexture, mLevel, mType.InternalFromat, width, height);
		glTextureParameteri(mTexture, GL_TEXTURE_MIN_FILTER, mLevel > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
		glTextureParameteri(mTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTextureParameteri(mTexture, GL_TEXTURE_WRAP_S, mType.Wrap);
		glTextureParameteri(mTexture, GL_TEXTURE_WRAP_T, mType.Wrap);
		glTextureParameteri(mTexture, GL_TEXTURE_WRAP_R, mType.Wrap);
	}

	OpenGLTextureCube::OpenGLTextureCube(const std::string & path)
		: mWidth(0), mHeight(0), mLevel(0), mTexture(0)
	{
		// Parameters
		static constexpr uint32_t cubemapSize = 1024;

		// Unfiltered environment cube map (temporary).
		OpenGLTextureCube envCubeUnfiltered(cubemapSize, cubemapSize, Texture::Description{ Texture::Format::RGBA16F, Texture::Wrap::Clamp, Texture::Filter::Linear });
		// Load & convert Equirectangular Environment Map to a Cubemap texture.
		{
			OpenGLShader equirectangleToCubeShader("assets/shaders/equirect2cube_cs.glsl");
			OpenGLTexture2D envEquirect(path, false, Texture::Wrap::Clamp, Texture::Filter::Linear);

			equirectangleToCubeShader.Bind();
			envEquirect.Bind(1);
			glBindImageTexture(0, envCubeUnfiltered.RendererID(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
			glDispatchCompute(envCubeUnfiltered.Width() / 32, envCubeUnfiltered.Height() / 32, 6);
		}
		glGenerateTextureMipmap(envCubeUnfiltered.RendererID());

		// Compute pre-filtered specular environment map.
		{
			OpenGLShader spmapShader("assets/shaders/spmap_cs.glsl");
			this->Create(cubemapSize, cubemapSize, Texture::Description{ Texture::Format::RGBA16F, Texture::Wrap::Clamp, Texture::Filter::Linear });

			// Copy unfiltered texture to the current
			glCopyImageSubData(envCubeUnfiltered.RendererID(), GL_TEXTURE_CUBE_MAP, 0, 0, 0, 0,
								this->RendererID(), GL_TEXTURE_CUBE_MAP, 0, 0, 0, 0,
								this->Width(), this->Height(), 6);

			spmapShader.Bind();
			envCubeUnfiltered.Bind();

			// Pre-filter rest of the mip chain.
			const float deltaRoughness = 1.0f / std::max(float(mLevel - 1), 1.0f);
			for (int level = 1, size = cubemapSize / 2; level <= mLevel; level++, size /= 2)
			{
				const GLuint numGroups = std::max(1, size / 32);
				glBindImageTexture(0, this->RendererID(), level, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
				glProgramUniform1f(spmapShader.RendererID(), 0, level * deltaRoughness);
				glDispatchCompute(numGroups, numGroups, 6);
			}
		}
	}

	OpenGLTextureCube::~OpenGLTextureCube()
	{
		glDeleteTextures(1, &mTexture);
	}

	void OpenGLTextureCube::BindImageTexture(bool layered)
	{
		glBindImageTexture(0, mTexture, 0, layered ? GL_TRUE : GL_FALSE, 0, GL_WRITE_ONLY, mType.InternalFromat);
	}

	void OpenGLTextureCube::Bind(uint32_t slot) const
	{
		glBindTextureUnit(slot, mTexture);
	}

	uint32_t OpenGLTextureCube::MipLevelCount() const
	{
		return uint32_t();
	}

	void OpenGLTextureCube::SetData(void * data, uint32_t size)
	{

	}

}


