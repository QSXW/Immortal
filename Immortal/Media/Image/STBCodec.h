#pragma once

#include <cstdint>

#include "Media/Interface/Codec.h"
#include "Media/External/stb_image/stb_image.h"

namespace Immortal
{
namespace Vision
{

class STBCodec : public Interface::Codec
{
public:
    using Super = Interface::Codec;

public:
    STBCodec()
    {

    }

    virtual ~STBCodec()
    {
        if (data)
        {
            stbi_image_free(data);
        }
    }

    virtual CodecError Decode(const std::vector<uint8_t> &buf) override
    {
        int depth = 0;
        data = stbi_load_from_memory(
                     buf.data(),
                     static_cast<int>(buf.size()),
                    &static_cast<int>(desc.width),
                    &static_cast<int>(desc.height),
                    &static_cast<int>(depth),
                    4
                    );

        if (!data)
        {
            return CodecError::CorruptedBitstream;
        }
        desc.format = Format::RGBA8;

        return CodecError::Succeed;
    }

    virtual void Encode()
    {

    }

    virtual uint8_t *Data() const
    {
        return data;
    }

    virtual void Swap(void *ptr)
    {
        void *tmp = nullptr;
        tmp  = ptr;
        ptr  = data;
        data = reinterpret_cast<stbi_uc *>(tmp);
    }

private:
    stbi_uc *data = nullptr;
};

}
}
