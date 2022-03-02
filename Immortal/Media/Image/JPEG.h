#pragma once

#ifndef JPEG_CODEC_H__

#include "Core.h"
#include "Media/Interface/Codec.h"

#include <bit>

namespace Immortal
{
namespace Vision
{

class IMMORTAL_API JpegCodec : public Interface::Codec
{
public:
    enum MarkerType
    {
        SOI  = 0xD8,
        EOI  = 0xD9,
        SOS  = 0xDA,
        DQT  = 0xDB,
        DNL  = 0xDC,
        DRI  = 0xDD,
        DHP  = 0xDE,
        EXP  = 0xDF,
        APP0 = 0xE0,
        APP1 = 0xE1,
        APPF = 0xEF,
        SOF0 = 0xC0, // Baseline DCT           
        SOF1 = 0xC1, // Extended sequential DCT
        SOF2 = 0xC2, // Progressive DCT
        DHT  = 0xC4,
    };

    struct Word
    {
        Word(const uint8_t *data)
        {
            bytes[1] = data[0];
            bytes[0] = data[1];
        }

        operator uint16_t() const
        {
            return word;
        }

        union
        {
            uint8_t bytes[2];
            uint16_t word;
        };
    };

    struct Component
    {
        uint8_t samplingFactor;
        uint8_t qtSelector;
        uint8_t htSelector;
    };

public:
    JpegCodec(const std::vector<uint8_t> &buffer);

    void ParseHeader(const std::vector<uint8_t> &buffer);

    template <class T>
    void ParseMarker(const uint8_t **data, T &&process)
    {
        auto *ptr = *data + 1;
        uint16_t length = Word{ ptr };
        *data += length;
        process(ptr + sizeof(uint16_t));
    }

private:
    void ParseDQT(const uint8_t *data);

    void ParseSOF(const uint8_t *data);

private:
    uint8_t bitDepth;
    std::vector<Component> components;
};

}
}

#endif
