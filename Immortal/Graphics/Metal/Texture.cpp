#include "Texture.h"
#include "Device.h"

namespace Immortal
{
namespace Metal
{

Texture::Texture() :
    Super{},
    Handle<MTL::Texture>{}
{

}

Texture::Texture(Device *device, Format format, uint32_t width, uint32_t height, uint16_t mipLevels, uint16_t arrayLayers, TextureType type) :
    Super{},
    Handle<MTL::Texture>{}
{
	SetMeta(format, width, height, mipLevels, arrayLayers);

    MTL::TextureDescriptor *descriptor = MTL::TextureDescriptor::alloc()->init();
    descriptor->setPixelFormat(format);
    descriptor->setWidth(width);
    descriptor->setHeight(height);
    descriptor->setMipmapLevelCount(mipLevels);
    descriptor->setArrayLength(arrayLayers);
    descriptor->setSampleCount(1);
    descriptor->setTextureType(arrayLayers > 1 ? MTL::TextureTypeCube : MTL::TextureType2D);
    descriptor->setAllowGPUOptimizedContents(true);

    MTL::ResourceOptions resourceOptions = MTL::ResourceStorageModePrivate;

    MTL::TextureUsage usage = MTL::TextureUsageShaderRead;
    if (type & TextureType::Storage)
    {
        usage |= MTL::TextureUsageShaderWrite;
    }
    if (type & TextureType::ColorAttachment || type & TextureType::DepthStencilAttachment)
    {
        usage |= MTL::TextureUsageRenderTarget;
    }
    descriptor->setUsage(usage);

    descriptor->setResourceOptions(resourceOptions);

    handle = device->GetHandle()->newTexture(descriptor);
    descriptor->release();
}

Texture::Texture(MTL::Texture *texture) :
    Super{},
    Handle<MTL::Texture>{ texture->retain() }
{
    SetMeta(handle->width(), handle->height(), handle->mipmapLevelCount(), handle->arrayLength());
}

Texture::~Texture()
{

}

}
}
