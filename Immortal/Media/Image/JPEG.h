#pragma once

#ifndef JPEG_CODEC_H__

#include "Core.h"
#include "Media/Interface/Codec.h"
#include "Media/Common/BitTracker.h"
#include "Memory/Allocator.h"

namespace Immortal
{
namespace Vision
{

struct Block
{
    uint16_t stride;
    uint8_t *data;
};

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

    struct HuffTable
    {
#define HUFFVAL_SIZE 257
        uint8_t bits[17];
        uint8_t huffval[HUFFVAL_SIZE];
    };

    struct HuffCodec
    {
        HuffCodec() = default;

        HuffCodec(const HuffTable &huffTable);

        int32_t MINCODE[17];
        int32_t MAXCODE[17];
        int32_t VALPTR[17];
    };

    struct SamplingFactor
    {
        int8_t horizontal;
        int8_t vertical;
    };

    struct Component
    {
        uint16_t x, y;
        int16_t width;
        int16_t height;
        uint8_t qtSelector;
        uint8_t htSelector;
        SamplingFactor sampingFactor;
    };

public:
    JpegCodec(const std::vector<uint8_t> &buffer);

    ~JpegCodec();

    void ParseHeader(const std::vector<uint8_t> &buffer);

    virtual CodecError Decode() override;

    template <class T>
    void ParseMarker(const uint8_t **data, T &&process);

private:
    void ParseAPP(const uint8_t *data);

    void ParseDQT(const uint8_t *data);

    void ParseDHT(const uint8_t *data);

    void ParseDRI(const uint8_t *data);

    void ParseSOF(const uint8_t *data);

    void ParseSOS(const uint8_t *data);

    void InitDecodedPlaneBuffer();

    void DecodeMCU();

    void DecodeBlock(Block *block);

private:
    std::vector<Component> components;

    std::array<Block, 8> blocks;

    uint8_t *buffer = nullptr;

    std::array<std::array<uint8_t, 128>, 4> quantizationTables;

    std::array<HuffCodec, 4> dctbl;
    std::array<HuffCodec, 4> actbl;

    BitTracker bitTracker;

    bool isProgressive = false;
    bool restartInterval = false;

    uint32_t mcuNumber = 0;

    uint8_t blockSize = 0;

    uint8_t bitDepth;

#ifndef JPEG_CODEC_MINIMAL
    struct {
        std::vector<uint8_t> external;
        struct {
            uint8_t identifier[5];
            uint8_t majorRevisionNum;
            uint8_t minorRevisionNum;
            uint8_t units;
            int16_t horizontalDensity;
            int16_t verticalDensity;
            uint8_t thumbnailWidth;
            uint8_t thumbnailHeight;
        } jfif;
    } application;
#endif
};

template <class T>
inline void JpegCodec::ParseMarker(const uint8_t **data, T &&process)
{
    auto *ptr = *data + 1;
    uint16_t length = Word{ ptr };
    *data += length;
    process(ptr + sizeof(uint16_t));
}

}
}

#endif
