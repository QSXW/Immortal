#include "impch.h"
#include "Texture.h"

#include "Renderer.h"
#include "Platform/OpenGL/OpenGLTexture.h"

namespace Immortal {

	Ref<Texture2D> Texture2D::Create(uint32_t width, uint32_t height)
	{
		return InstantiateGrphicsPrimitive<Texture2D, OpenGLTexture2D, OpenGLTexture2D, OpenGLTexture2D>(width, height);
	}

	Ref<Texture2D> Texture2D::Create(const std::string & filepath)
	{
		return InstantiateGrphicsPrimitive<Texture2D, OpenGLTexture2D, OpenGLTexture2D, OpenGLTexture2D>(filepath);
	}

	Ref<Texture2D> Texture2D::Create(const uint32_t width, const uint32_t height, Texture::Description &spec, int levels)
	{
		return InstantiateGrphicsPrimitive<Texture2D, OpenGLTexture2D, OpenGLTexture2D, OpenGLTexture2D>(width, height, spec, levels);
	}

	Ref<Texture2D> Texture2D::Create(const std::string & path, Texture::Wrap wrap, Texture::Filter filter)
	{
		return InstantiateGrphicsPrimitive<Texture2D, OpenGLTexture2D, OpenGLTexture2D, OpenGLTexture2D>(path, wrap, filter);
	}

	Ref<Texture2D> Texture2D::Create(UINT32 width, UINT32 height, const void * data, Texture::Description & spec)
	{
		return InstantiateGrphicsPrimitive<Texture2D, OpenGLTexture2D, OpenGLTexture2D, OpenGLTexture2D>(width, height, data, spec);
	}

	Ref<Texture2D> Texture2D::Create(const std::string & path, bool flip, Texture::Wrap wrap, Texture::Filter filter)
	{
		return InstantiateGrphicsPrimitive<Texture2D, OpenGLTexture2D, OpenGLTexture2D, OpenGLTexture2D>(path, flip, wrap, filter);
	}

	Ref<TextureCube> TextureCube::Create(const uint32_t width, const uint32_t height, Texture::Description &spec, int levels)
	{
		return InstantiateGrphicsPrimitive<TextureCube, OpenGLTextureCube, OpenGLTextureCube, OpenGLTextureCube>(width, height, spec, levels);
	}

	Ref<TextureCube> TextureCube::Create(const std::string & filepath)
	{
		return InstantiateGrphicsPrimitive<TextureCube, OpenGLTextureCube, OpenGLTextureCube, OpenGLTextureCube>(filepath);
	}

}