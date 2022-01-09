#pragma once

#include <stdint.h>

#include "Decoder.h"
#include "stb_image/stb_image.h"

namespace Immortal
{
namespace Media
{

class STBCodec : public Decoder
{
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
        data = stbi_load_from_memory(
                    buf.data(),
                     static_cast<int>(buf.size()),
                    &static_cast<int>(desc.Width),
                    &static_cast<int>(desc.Height),
                    &static_cast<int>(desc.Depth),
                    4
                    );

        if (!data)
        {
            return CodecError::CorruptedBitstream;
        }

        FillUpDescription();

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
