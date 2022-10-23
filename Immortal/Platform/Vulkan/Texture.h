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

class Texture : public virtual SuperTexture
{
public:
    using Super = SuperTexture;

public:
    Texture(Device *device, const std::string &filepath, const Description &description = Description{});

    Texture(Device *device, uint32_t width, uint32_t height, const void *data, const Description &description, uint32_t layer = 1);

    ~Texture();

    void Setup(const Description &description, uint32_t size, const void *data = nullptr, uint32_t layer = 1);

    virtual operator uint64_t() const override;

    virtual void Update(const void *data, uint32_t pitchX = 0) override;

    virtual void Blit() override;

    virtual void As(DescriptorBuffer *descriptorBuffer, size_t index) override;

    virtual bool operator==(const Texture::Super &super) const override;

public:
    ImageDescriptor &DescriptorInfo()
    {
        return descriptor;
    }

    operator VkImage() const
    {
        return *image;
    }

private:
    void InternalUpdate(const void *data, uint32_t pitchX = 0);

    void Synchronize(VkImageLayout newLayout);

    void GenerateMipMaps();

private:
    Device *device{ nullptr };

    Description desc;

    URef<Sampler> sampler;

    URef<Image> image;

    URef<ImageView> view;

    URef<DescriptorSet> descriptorSet;

    ImageDescriptor descriptor{};

public:
    VkImageLayout Layout{ VK_IMAGE_LAYOUT_UNDEFINED };
};

class TextureCube : public Vulkan::Texture, public SuperTextureCube
{
public:
    using Super = SuperTextureCube;

public:
    TextureCube(Device *device, uint32_t width, uint32_t height, const Description &desc = {});
};

}
}
