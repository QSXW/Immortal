#pragma once

#include "Core.h"

namespace Immortal
{

class Buffer;
class Texture;
class Sampler;
class IMMORTAL_API DescriptorSet : public IObject
{
public:
    virtual ~DescriptorSet() = default;

    virtual void Set(uint32_t slot, Buffer *buffer) = 0;

    virtual void Set(uint32_t slot, Texture *texture) = 0;

    virtual void Set(uint32_t slot, Sampler *sampler) = 0;
};

using SuperDescriptorSet = DescriptorSet;

}
