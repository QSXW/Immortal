#include "STBCodec.h"

namespace Immortal
{
namespace Vision
{

STBCodec::~STBCodec()
{
    if (data)
    {
        stbi_image_free(data);
    }
}

CodecError STBCodec::Decode(const std::vector<uint8_t> &buf)
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

uint8_t *STBCodec::Data() const
{
    return data;
}

void STBCodec::Swap(void *ptr)
{
    void *tmp = nullptr;
    tmp  = ptr;
    ptr  = data;
    data = reinterpret_cast<stbi_uc *>(tmp);
}

}
}
