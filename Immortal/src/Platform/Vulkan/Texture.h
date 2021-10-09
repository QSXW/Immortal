#pragma once

#include "Common.h"
#include "Render/Texture.h"

#include "RenderContext.h"

namespace Immortal
{
namespace Vulkan
{

class Texture : public SuperTexture2D
{
public:
    Texture(RenderContext *context, const std::string &filepath);

    ~Texture();
private:

};

}
}
