#pragma once

#include "Common.h"
#include "Render/Texture.h"

#include "RenderContext.h"

#include "Image.h"
#include "ImageView.h"
#include "Sampler.h"
#include "DescriptorSet.h"

namespace Immortal
{
namespace Vulkan
{

class Texture : public SuperTexture
{
public:
    using Super = SuperTexture;

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

    void Setup(const Description &description, uint32_t size, const void *data = nullptr);

    void ConvertType(VkImageCreateInfo &dst, const Description &src)
    {
        dst.format = src.BaseFromat<VkFormat>();
    }

    void SetupSampler(const Description &desc)
    {
        sampler = Sampler{ device, desc };
    }

    void SetupImageView(VkFormat format)
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

    virtual operator uint64_t() const override
    {
        return *descriptorSet;
    }

    virtual void As(Descriptor *descriptor, size_t index) override;

    virtual void Map(uint32_t slot = 0) override
    {

    }

    ImageDescriptor &DescriptorInfo()
    {
        return descriptor;
    }

private:
    Device *device{ nullptr };

    Sampler sampler;

    std::unique_ptr<ImageView> view;

    VkImage image{ VK_NULL_HANDLE };

    VkImageLayout layout{ VK_IMAGE_LAYOUT_UNDEFINED };

    VkDeviceMemory deviceMemory{ VK_NULL_HANDLE };

    std::unique_ptr<DescriptorSet> descriptorSet;

    ImageDescriptor descriptor{};
};

}
}
