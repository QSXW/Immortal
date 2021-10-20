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

    void ConvertType(VkImageCreateInfo &dst, Description &src)
    {
        switch (src.Format)
        {
        case SuperTexture::Format::RGBA:
        case SuperTexture::Format::RGBA8:    
            dst.format = VK_FORMAT_R8G8B8A8_UNORM;
            break;
            
        case SuperTexture::Format::BGRA8:
            dst.format = VK_FORMAT_B8G8R8A8_UNORM;
            break;

        default:
            dst.format = VK_FORMAT_R8G8B8A8_UNORM;
            break;
        }
    }

    void INITSampler(const Description &desc)
    {
        VkSamplerCreateInfo createInfo{};
        createInfo.sType        = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        createInfo.magFilter    = VK_FILTER_NEAREST;
        createInfo.minFilter    = VK_FILTER_NEAREST;
        createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

        if (desc.Filter == Filter::Bilinear)
        {
            createInfo.magFilter = VK_FILTER_LINEAR;;
            createInfo.minFilter = VK_FILTER_LINEAR;
        }
        if (desc.Anisotropic)
        {
            createInfo.anisotropyEnable = VK_TRUE;
            createInfo.maxAnisotropy    = device->Get<PhysicalDevice>().Properties.limits.maxSamplerAnisotropy;
        }
        if (desc.Wrap == Wrap::Clamp)
        {
            createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
            createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
            createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        }
        if (desc.Wrap == Wrap::Mirror)
        {
            createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
            createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
            createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        }
        if (desc.Wrap == Wrap::BorderColor)
        {
            createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
            createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
            createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        }

        sampler = Sampler{ device, createInfo };
    }

    void INITImageView()
    {
        view = std::make_unique<ImageView>(image.get(), VK_IMAGE_VIEW_TYPE_2D);
    }

    void INITDescriptor();

    virtual uint64_t Handle() const override
    {
        return rcast<uint64_t>(descriptorSet);
    }

private:
    Device *device{ nullptr };

    uint32_t width{ 0 };

    uint32_t height{ 0 };

    uint32_t mipLevels{ 1 };

    std::unique_ptr<Image> image;

    std::unique_ptr<ImageView> view;

    Sampler sampler;

    VkImageLayout layout{ VK_IMAGE_LAYOUT_UNDEFINED };

    VkDeviceMemory deviceMemory;

    VkDescriptorSet descriptorSet;

    VkDescriptorSetLayout descriptorSetLayout;

    VkPipelineLayout pipelineLayout;
};

}
}
