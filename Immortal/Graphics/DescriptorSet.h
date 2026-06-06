#pragma once

#include "Core.h"
#include "Shared/IObject.h"

namespace Immortal
{

class Buffer;
class BufferView;
class Texture;
class Sampler;
class IMMORTAL_API DescriptorSet : public IObject
{
public:
    virtual ~DescriptorSet() = default;

    virtual void Set(uint32_t slot, Buffer *buffer) = 0;

    virtual void Set(uint32_t slot, BufferView *view) {}

    virtual void Set(uint32_t slot, Texture *texture) = 0;

    /** Optional: bind a single mip slice as storage image (IBL prefilter). Default no-op. */
    virtual void SetUavMip(uint32_t slot, Texture *texture, uint32_t mipSlice)
    {
        (void)slot;
        (void)texture;
        (void)mipSlice;
    }

    virtual void Set(uint32_t slot, Sampler *sampler) = 0;
};

using SuperDescriptorSet = DescriptorSet;

}
