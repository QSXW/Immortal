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
    int depth  = 0;
    int width  = static_cast<int>(desc.width);
    int height = static_cast<int>(desc.height);
    data = stbi_load_from_memory(
                    buf.data(),
                    static_cast<int>(buf.size()),
                    &width,
                    &height,
                    &depth,
                    4
                );

    if (!data)
    {
        return CodecError::CorruptedBitstream;
    }

    desc.format = Format::RGBA8;
    desc.width  = width;
    desc.height = height;

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
