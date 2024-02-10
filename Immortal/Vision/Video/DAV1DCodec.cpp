#include "DAV1DCodec.h"

#if HAVE_DAV1D
#include <dav1d/dav1d.h>
#include <dav1d/version.h>

#include "Vision/Processing/ColorSpace.h"
#include "Vision/Image/MFXJpegCodec.h"

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

CodecError DAV1DCodec::Decode(const CodedFrame &codedFrame)
{
    int res = 0;
    Dav1dData dav1dData{};
    Dav1dPicture dav1dPicture{};

    auto ptr = dav1d_data_create(&dav1dData, codedFrame.buffer.size());

    memcpy(ptr, codedFrame.buffer.data(), codedFrame.buffer.size());
    dav1dData.m.timestamp = codedFrame.timestamp;
    dav1dData.m.offset    = codedFrame.offset;

    do {
        if ((res = dav1d_send_data(handle, &dav1dData)) < 0)
        {
            if (res != DAV1D_ERR(EAGAIN))
            {
                dav1d_data_unref(&dav1dData);
                LOG::ERR("Error on decoding frame: {}", strerror(DAV1D_ERR(res)));
                return CodecError::CorruptedBitstream;
            }
        }

        memset(&dav1dPicture, 0, sizeof(Dav1dPicture));
        if ((res = dav1d_get_picture(handle, &dav1dPicture)) < 0)
        {
            if (res != DAV1D_ERR(EAGAIN))
            {
                LOG::ERR("Error on decoding frame: {}", strerror(DAV1D_ERR(res)));
            }
            if (dav1dData.sz == 0)
            {
                return CodecError::Repeat;
            }
        }
        else
        {
            picture = Picture{ dav1dPicture.p.w, dav1dPicture.p.h, Format::RGBA8 };

            CVector<uint8_t> dst{};
            dst.x = picture.Data();

            CVector<uint8_t> src{};
            src.x = (uint8_t *)dav1dPicture.data[0];
            src.y = (uint8_t *)dav1dPicture.data[1];
            src.z = (uint8_t *)dav1dPicture.data[2];
            src.linesize[0] = dav1dPicture.stride[0];
            src.linesize[1] = dav1dPicture.stride[1];

            YUV420PToRGBA8(dst, src, picture.desc.width, picture.desc.height);

            dav1d_picture_unref(&dav1dPicture);
        }      
    } while (dav1dData.sz > 0);

    dav1d_data_unref(&dav1dData);
    return CodecError::Success;
}

uint8_t * DAV1DCodec::Data() const
{
    return picture.Data();
}

Picture DAV1DCodec::GetPicture() const
{
    return picture;
}

}
}

#endif
