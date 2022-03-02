#include "JPEG.h"

namespace Immortal
{
namespace Vision
{

JpegCodec::JpegCodec(const std::vector<uint8_t> &buffer)
{
    ParseHeader(buffer);
}

void JpegCodec::ParseHeader(const std::vector<uint8_t> &buffer)
{
    ThrowIf(buffer[0] != 0xff && buffer[1] != 0xd8, "Not a Jpeg file");

    auto *end = buffer.data() + buffer.size();
    for (auto ptr = &buffer[2]; ptr < end; ptr++)
    {
        if (*ptr == 0xff)
        {
            switch (*++ptr)
            {
            case MarkerType::DHT:
                break;

            case MarkerType::DQT:
                ParseMarker(&ptr, [&] (auto payload) { JpegCodec::ParseDQT(payload); });
                break;

            case MarkerType::SOS:
                LOG::DEBUG("Found the start of scan. Parsing completed!");
                ptr = end;
                break;

            case MarkerType::SOF0:
            case MarkerType::SOF1:
            case MarkerType::SOF2:
                ParseMarker(&ptr, [&](auto payload) { JpegCodec::ParseSOF(payload); });
                break;

            case MarkerType::APP0:
            case MarkerType::APP1:
                break;

            default:
                LOG::WARN("Unsupport Jpeg Marker Type: {}", *ptr);
                break;
            }
        }
    }
}


inline void JpegCodec::ParseDQT(const uint8_t *data)
{

}

inline void JpegCodec::ParseSOF(const uint8_t *data)
{
    bitDepth    = data[0];
    desc.height = Word{ &data[1] };
    desc.width  = Word{ &data[3] };

    components.resize(data[5]);
    auto ptr = &data[6];
    for (size_t i = 0; i < components.size(); i++, ptr += 3)
    {
        auto index = ptr[0] - 1;
        components[index].samplingFactor = ptr[1];
        components[index].qtSelector = ptr[2];
    }
}

}
}
