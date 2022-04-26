#include "DAV1DCodec.h"

#if HAVE_DAV1D
#include <dav1d/dav1d.h>
#include <dav1d/version.h>

#include "Processing/ColorSpace.h"

namespace Immortal
{
namespace Vision
{

static inline Format SelectFormat(Dav1dPixelLayout layout)
{
    switch (layout)
    {
    case DAV1D_PIXEL_LAYOUT_I420: return Format::YUV420P;
    case DAV1D_PIXEL_LAYOUT_I422: return Format::YUV422P;
    case DAV1D_PIXEL_LAYOUT_I444: return Format::YUV444P;
    default:
        return Format::None;
    }
}

DAV1DCodec::DAV1DCodec() :
    handle{ nullptr },
    version{ nullptr }
{
    CheckVersion();

    Dav1dSettings settings;
    dav1d_default_settings(&settings);
    
    if (dav1d_open(&handle, &settings))
    {
        throw RuntimeException("Failed to open dav1d context");
    }
}

DAV1DCodec::~DAV1DCodec()
{
    if (handle)
    {
        dav1d_close(&handle);
    }
}

void DAV1DCodec::CheckVersion()
{
    version = dav1d_version();
}

CodecError DAV1DCodec::Decode(CodedFrame *codedFrame)
{
    int res = 0;
    Dav1dData dav1dData{};
    auto ptr = dav1d_data_create(&dav1dData, codedFrame->buffer.size());

    memcpy(ptr, codedFrame->buffer.data(), codedFrame->buffer.size());
    dav1dData.m.timestamp = codedFrame->timestamp;
    dav1dData.m.offset    = codedFrame->offset;

    if ((res = dav1d_send_data(handle, &dav1dData)) < 0)
    {
        throw RuntimeException("Failed to consume data");
    }

    Dav1dPicture picture{};
    if ((res = dav1d_get_picture(handle, &picture)) < 0)
    {
        return CodecError::Repeat;
    }
    
    desc.format = Format::RGBA8;
    desc.width  = picture.p.w;
    desc.height = picture.p.h;
    data = new uint8_t[desc.Size()];

    ColorSpace::Vector<uint8_t> dst{};
    dst.r = &data[0];
    dst.g = &data[1];
    dst.b = &data[2];
    dst.a = &data[3];

    ColorSpace::Vector<uint8_t> src{};
    src.x = (uint8_t *)picture.data[0];
    src.y = (uint8_t *)picture.data[1];
    src.z = (uint8_t *)picture.data[2];

    ColorSpace::YUV420PToRGBA8(dst, src, desc.width, desc.height);

    return CodecError::Succeed;
}

uint8_t * DAV1DCodec::Data() const
{
    return data;
}

}
}

#endif
