#pragma once

#include <cstdint>
#include <cstdlib>
#include <cstring>

#include <string>
#include <memory>

#include "Vision/Codec.h"

namespace Immortal
{
namespace Vision
{

class BMPCodec : public Interface::Codec
{
public:
    using Super = Interface::Codec;

public:
    BMPCodec() :
        Super{}
    {
        memset(&identifer, 0, HeaderSize());
    }

    virtual ~BMPCodec()
    {

    }

    virtual CodecError Decode(const CodedFrame &codedFrame);

    size_t HeaderSize()
    {
        return reinterpret_cast<uint8_t *>(&importantColours) - reinterpret_cast<uint8_t *>(&identifer);
    }

    bool Write(const std::string &filepath, int width, int height, int depth, uint8_t *data, int align = 0);

private:
    #pragma pack(push, 1)
    uint16_t identifer;
    uint32_t fileSize;
    uint16_t RESERVER_ST;
    uint16_t RESERVER_ND;
    uint32_t offset;
    uint32_t informationSize;
    int32_t  width;
    int32_t  height;
    uint16_t planes;
    uint16_t bitsPerPixel;
    uint32_t compressionType;
    uint32_t imageSize;
    int32_t  horizontalResolution;
    int32_t  verticalResolution;
    uint32_t coloursNum;
    uint32_t importantColours;
    uint16_t empty;
    #pragma pack(pop)
};

}
}
