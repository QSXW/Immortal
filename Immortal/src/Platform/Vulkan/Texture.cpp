#include "Texture.h"

#include "Render/Frame.h"

namespace Immortal
{
namespace Vulkan
{

Texture::Texture(RenderContext *context, const std::string &filepath) :
    device{ context->Get<Device*>() }
{
    Frame frame{ filepath };

    width     = frame.Width();
    height    = frame.Height();
    mipLevels = 1;

    VkBufferCreateInfo createInfo{};
    createInfo.pNext       = nullptr;
    createInfo.size        = frame.Size();
    createInfo.usage       = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
}


Texture::~Texture()
{

}
}
}
