#include "impch.h"
#include "Texture.h"

#include "Render.h"
#include "Platform/OpenGL/Texture.h"
#include "..\Platform\D3D12\Texture.h"

namespace Immortal
{

std::shared_ptr<Texture2D> Texture2D::Create(uint32_t width, uint32_t height)
{
	return CreateSuper<Texture2D, OpenGL::Texture2D, OpenGL::Texture2D, OpenGL::Texture2D>(width, height);
}

std::shared_ptr<Texture2D> Texture2D::Create(const std::string & filepath)
{
	return CreateSuper<Texture2D, OpenGL::Texture2D, OpenGL::Texture2D, OpenGL::Texture2D>(filepath);
}

std::shared_ptr<Texture2D> Texture2D::Create(const uint32_t width, const uint32_t height, Texture::Description &spec, int levels)
{
	return CreateSuper<Texture2D, OpenGL::Texture2D, OpenGL::Texture2D, OpenGL::Texture2D>(width, height, spec, levels);
}

std::shared_ptr<Texture2D> Texture2D::Create(const std::string & path, Texture::Wrap wrap, Texture::Filter filter)
{
	return CreateSuper<Texture2D, OpenGL::Texture2D, OpenGL::Texture2D, OpenGL::Texture2D>(path, wrap, filter);
}

std::shared_ptr<Texture2D> Texture2D::Create(UINT32 width, UINT32 height, const void * data, Texture::Description & spec)
{
	return CreateSuper<Texture2D, OpenGL::Texture2D, OpenGL::Texture2D, OpenGL::Texture2D>(width, height, data, spec);
}

std::shared_ptr<Texture2D> Texture2D::Create(const std::string & path, bool flip, Texture::Wrap wrap, Texture::Filter filter)
{
	return CreateSuper<Texture2D, OpenGL::Texture2D, OpenGL::Texture2D, OpenGL::Texture2D>(path, flip, wrap, filter);
}

std::shared_ptr<TextureCube> TextureCube::Create(const uint32_t width, const uint32_t height, Texture::Description &spec, int levels)
{
	return CreateSuper<TextureCube, OpenGL::TextureCube, OpenGL::TextureCube, OpenGL::TextureCube>(width, height, spec, levels);
}

std::shared_ptr<TextureCube> TextureCube::Create(const std::string & filepath)
{
	return CreateSuper<TextureCube, OpenGL::TextureCube, OpenGL::TextureCube, OpenGL::TextureCube>(filepath);
}

}
