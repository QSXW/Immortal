/**
 * Copyright (C) 2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#include "Graphics/Buffer.h"

namespace Immortal
{
namespace Light
{

class Buffer : public SuperBuffer
{
public:
    using Super = SuperBuffer;

public:
    Buffer(uint32_t size, Type type);

    Buffer(uint32_t size, const void* data, Type type);

    virtual ~Buffer() override;

    virtual void Update(uint32_t size, const void* data, uint32_t offset = 0) override;

protected:
    URef<uint8_t> buffer;
};

}
}
