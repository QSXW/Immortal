#include "Texture.h"

#include "Render/Frame.h"

namespace Immortal
{
namespace Vulkan
{

Texture::Texture(RenderContext *context, const std::string &filepath)
{
    Frame frame{ filepath };

    width     = frame.Width();
    height    = frame.Height();
    mipLevels = 1;

}


Texture::~Texture()
{

}
}
}
