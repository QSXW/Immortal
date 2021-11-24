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
    Texture(Device *device, const std::string &filepath);

    Texture(Device *device, uint32_t width, uint32_t height, const void *data, const Description &description);

    ~Texture();

    template <class T>
    T Get()
    {
        if (is_same<T, ImageView*>())
        {
            return view.get();
        }
    }

    void INIT(const Description &description, uint32_t size, const void *data = nullptr);

    void ConvertType(VkImageCreateInfo &dst, const Description &src)
    {
        dst.format = src.BaseFromat<VkFormat>();
    }

    void INITSampler(const Description &desc)
    {
        sampler = Sampler{ device, desc };
    }

    void INITImageView(VkFormat format)
    {
        view = std::make_unique<ImageView>(
            device,
            image,
            VK_IMAGE_VIEW_TYPE_2D,
            format,
            0,
            0,
            mipLevels,
            1
            );
    }

    void INITDescriptor();

    virtual uint64_t Descriptor() const override
    {
        return rcast<uint64_t>(descriptorSet);
    }

    virtual void Map(uint32_t slot = 0) override
    {
        if (binding)
        {
            return;
        }

        VkDescriptorImageInfo textureDescriptor{};
        textureDescriptor.imageView   = *view;
        textureDescriptor.sampler     = sampler;
        textureDescriptor.imageLayout = layout;

        VkWriteDescriptorSet writeDesc{};
        writeDesc.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDesc.pNext           = nullptr;
        writeDesc.dstBinding      = slot;
        writeDesc.dstSet          = descriptorSet;
        writeDesc.descriptorCount = 1;
        writeDesc.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        writeDesc.pImageInfo      = &textureDescriptor;

        device->UpdateDescriptorSets(1, &writeDesc, 0, nullptr);

        binding = true;
    }

private:
    Device *device{ nullptr };

    uint32_t width{ 0 };

    uint32_t height{ 0 };

    uint32_t mipLevels{ 1 };

    Sampler sampler;

    std::unique_ptr<ImageView> view;

    VkImage image{ VK_NULL_HANDLE };

    VkImageLayout layout{ VK_IMAGE_LAYOUT_UNDEFINED };

    VkDeviceMemory deviceMemory{ VK_NULL_HANDLE };

    VkDescriptorSet descriptorSet{ VK_NULL_HANDLE };

    VkDescriptorSetLayout descriptorSetLayout{ VK_NULL_HANDLE };

    bool binding{ false };
};

}
}
