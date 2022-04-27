#pragma once

#include "Common.h"
#include "Render/Texture.h"

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
    Texture(Device *device, const std::string &filepath, const Description &description = Description{});

    Texture(Device *device, uint32_t width, uint32_t height, const void *data, const Description &description);

    ~Texture();

    template <class T>
    T Get()
    {
        if constexpr (IsPrimitiveOf<VkImage, T>())
        {
            return *image;
        }
    }

    template <class T>
    T *GetAddress()
    {
        if (IsPrimitiveOf<ImageView, T>())
        {
            return view.get();
        }
    }

    void Setup(const Description &description, uint32_t size, const void *data = nullptr);

    void SetupSampler(const Description &desc)
    {
        sampler = Sampler{ device, desc };
    }

    void SetupImageView(VkFormat format)
    {
        view.reset(new ImageView{
            image.get(),
            VK_IMAGE_VIEW_TYPE_2D
            });
    }

    virtual operator uint64_t() const override
    {
        return *descriptorSet;
    }

    virtual void Update(void *data);

    virtual void As(Descriptor *descriptor, size_t index) override;

    virtual void Map(uint32_t slot = 0) override
    {

    }

    ImageDescriptor &DescriptorInfo()
    {
        return descriptor;
    }

    virtual bool operator==(const Texture::Super &super) const override
    {
        auto other = dcast<const Texture *>(&super);
        return image == other->image;
    }

private:
    Device *device{ nullptr };

    Sampler sampler;

    std::unique_ptr<Image> image;

    std::unique_ptr<ImageView> view;

    std::unique_ptr<DescriptorSet> descriptorSet;

    ImageDescriptor descriptor{};

public:
    VkImageLayout Layout{ VK_IMAGE_LAYOUT_UNDEFINED };
};

}
}
