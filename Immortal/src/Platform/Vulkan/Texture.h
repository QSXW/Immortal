#pragma once

#include "Common.h"
#include "Render/Texture.h"

#include "RenderContext.h"

#include "Image.h"
#include "ImageView.h"
#include "Sampler.h"

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
    Device *device{ nullptr };

    uint32_t width{ 0 };

    uint32_t height{ 0 };

    uint32_t mipLevels{ 0 };

    std::unique_ptr<Image> image;

    std::unique_ptr<ImageView> view;

    Sampler sampler;

    VkImageLayout layout{ VK_IMAGE_LAYOUT_UNDEFINED };
};

}
}
