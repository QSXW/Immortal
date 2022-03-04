#include "JPEG.h"

namespace Immortal
{
namespace Vision
{

static inline Format SelectFormat(JpegCodec::SamplingFactor &sampingFactor)
{
    switch (sampingFactor.horizontal)
    {
    case 1:
        return Format::YUV444P;

    case 2:
        if (sampingFactor.vertical == 2)
        {
            return Format::YUV420P;
        }
        if (sampingFactor.vertical == 1)
        {
            return Format::YUV422P;
        }
        break;

    default:
        ThrowIf(true, "Incorrect Samping Factor");
        return Format::None;
    }
}

static inline uint16_t AlignToMCU(uint16_t v)
{
    auto padding = v & 0x7;
    return v + (padding ? 8 - padding : 0);
}

JpegCodec::JpegCodec(const std::vector<uint8_t> &buffer)
{
    ParseHeader(buffer);
}

JpegCodec::~JpegCodec()
{
    constexpr size_t align = 64;
    if (buffer)
    {
        AAllocator<uint8_t, align> allocator;
        allocator.deallocate(buffer, align);
    }
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
            case MarkerType::SOI:
                LOG::DEBUG("Detected marker type: SOI");
                break;

            case MarkerType::DHT:
                ParseMarker(&ptr, [&](auto payload) { JpegCodec::ParseDHT(payload); });
                break;

            case MarkerType::DQT:
                ParseMarker(&ptr, [&] (auto payload) { JpegCodec::ParseDQT(payload); });
                break;

            case MarkerType::SOS:
                ParseMarker(&ptr, [&](auto payload) { JpegCodec::ParseSOS(payload); });
                LOG::DEBUG("Found the start of scan. Parsing completed!");
                bitTracker = BitTracker{ ptr, (size_t)(end - ptr) };
                ptr = end;
                break;

            case MarkerType::SOF0:
            case MarkerType::SOF1:
            case MarkerType::SOF2:
                isProgressive = *ptr == MarkerType::SOF2;
                ParseMarker(&ptr, [&](auto payload) { JpegCodec::ParseSOF(payload); });
                break;

            case MarkerType::APP0:
            case MarkerType::APP1:
                ParseMarker(&ptr, [&](auto payload) { JpegCodec::ParseAPP(payload); });
                break;

            case MarkerType::DRI:
                ParseMarker(&ptr, [&](auto payload) { JpegCodec::ParseDRI(payload); });
                break;

            default:
                LOG::WARN("Unsupport Jpeg Marker Type: {}", *ptr);
                break;
            }
        }
    }
}

CodecError JpegCodec::Decode()
{
    InitDecodedPlaneBuffer();
    return CodecError::Succeed;
}

inline void JpegCodec::ParseAPP(const uint8_t *data)
{
#ifndef JPEG_CODEC_MINIMAL
    uint8_t type = data[-3];

    if (type > MarkerType::APP0 && type <= MarkerType::APPF)
    {
        LOG::DEBUG("Detected Application specified marker!");
        uint16_t length = Word{ &data[-2] } - 2;
        application.external.resize(length);
        memcpy(application.external.data(), data, length);
    }
    else
    {
        auto &jfif = application.jfif;
        *(uint32_t *)jfif.identifier = *(uint32_t *)data;
        jfif.identifier[4]     = data[4];
        jfif.majorRevisionNum  = data[5];
        jfif.minorRevisionNum  = data[6];
        jfif.units             = data[7];
        jfif.horizontalDensity = Word{ &data[8] };
        jfif.verticalDensity   = Word{ &data[10] };
        jfif.thumbnailWidth    = data[12];
        jfif.thumbnailHeight   = data[13];
    }
#endif
}

inline void JpegCodec::ParseDQT(const uint8_t *data)
{
    LOG::DEBUG("Parsing Quatization Table");
    uint16_t length = Word{ &data[-2] } - 2;
    size_t i = 0;

    do
    {
        uint8_t selector = data[i++];
        uint8_t precision = (selector & 0xf0) >> 4;
        uint8_t size = (precision + 1) * 64;
        auto &table = quantizationTables[selector & 0x0f];
        memcpy(table.data(), &data[i], size);
        i += size;
    } while (i < length);
}

inline void JpegCodec::ParseDHT(const uint8_t *data)
{
    LOG::DEBUG("Parsing Huffman Table");

    uint16_t length = Word{ &data[-2] } - 2;
    size_t i = 0;

    do
    {
        uint8_t selector = data[i++];
        uint8_t index = selector & 0x0f;
        auto &table = (0x10 & selector) ? actbl[index] : dctbl[index];

        HuffTable huffTable;
        CleanUpObject(&huffTable);

        size_t tableLength = 0;
        int j = 1;
        while (j < SL_ARRAY_LENGTH(huffTable.bits))
        {
            tableLength += huffTable.bits[j++] = data[i++];
        }
        memcpy(huffTable.huffval, &data[i], tableLength);
        i += tableLength;

        table = HuffCodec{ huffTable };
    } while (i < length);
}

inline void JpegCodec::ParseDRI(const uint8_t *data)
{
    mcuNumber = data[0];
    restartInterval = mcuNumber;
}

inline void JpegCodec::ParseSOF(const uint8_t *data)
{
    bitDepth    = data[0];
    desc.height = Word{ &data[1] };
    desc.width  = Word{ &data[3] };

    components.resize(data[5]);
    auto ptr = &data[6];

    SamplingFactor maxSampingFactor{ 0, 0 };
    for (size_t i = 0; i < components.size(); i++, ptr += 3)
    {
        auto index = ptr[0] - 1;
        uint8_t samplingFactor = ptr[1];

        components[index].sampingFactor.vertical   = (samplingFactor     ) & 0x0f;
        components[index].sampingFactor.horizontal = (samplingFactor >> 4) & 0x0f;
        components[index].qtSelector = ptr[2];

        maxSampingFactor.vertical   = std::max(maxSampingFactor.vertical, components[index].sampingFactor.vertical);
        maxSampingFactor.horizontal = std::max(maxSampingFactor.horizontal, components[index].sampingFactor.horizontal);
    }

    desc.format = SelectFormat(components[0].sampingFactor);

    for (size_t i = 0; i < components.size(); i++)
    {
        auto &component = components[i];
        component.width  = std::ceil(desc.width * ((float)component.sampingFactor.horizontal / (float)maxSampingFactor.horizontal));
        component.height = std::ceil(desc.height * ((float)component.sampingFactor.vertical / (float)maxSampingFactor.vertical));

        component.x = AlignToMCU(component.width);
        component.y = AlignToMCU(component.height);
        mcuNumber += component.x * component.y / 64;

        blockSize += component.sampingFactor.horizontal * component.sampingFactor.vertical;
    }
}

inline void JpegCodec::ParseSOS(const uint8_t *data)
{
    uint8_t componentNum = data[0];
    
    if (componentNum != components.size())
    {
        LOG::WARN("The component number of Start of Scan mismatch to Start of Frame's");
        if (componentNum > components.size())
        {
            components.resize(componentNum);
        }
    }
    
    auto ptr = &data[1];
    for (size_t i = 0; i < components.size(); i++, ptr += 2)
    {
        auto index = ptr[0] - 1;
        components[index].htSelector = ptr[1];
    }
}

void JpegCodec::InitDecodedPlaneBuffer()
{
    constexpr size_t align = 64;

    AAllocator<uint8_t, align> allocator;
    size_t size = 0;

    auto planes = desc.format.ComponentCount();
    std::array<uint32_t, 4> offsets{ 0 };
    for (size_t i = 0; i < planes; i++)
    {
        offsets[i] = size;
        size += SLALIGN(components[i].x * components[i].y, align);
    }
    buffer = allocator.allocate(size);
   
    size_t blockIndex = 0;
    for (size_t i = 0, j = 0; i < planes; i++)
    {
        uint32_t offset = offsets[i];
        uint32_t stride = components[i].x * 8;
        for (size_t v = 0; v < components[i].sampingFactor.vertical; v++)
        {
            for (size_t h = 0; h < components[i].sampingFactor.horizontal; h++)
            {
                blocks[blockIndex].stride = stride;
                blocks[blockIndex].data = &buffer[offset + h * 8 + v * stride];
                blockIndex++;
            }
        }
    }
}

void JpegCodec::DecodeMCU()
{
    for (size_t i = 0; i < mcuNumber; i++)
    {
        for (size_t j = 0; j < blockSize; j++)
        {
            DecodeBlock(&blocks[j]);
        }
    }
}

__forceinline void JpegCodec::DecodeBlock(Block *block)
{

}

static void GenerateHuffSize(const uint8_t *BITS, int32_t *HUFFSIZE, int32_t *lastk)
{
    int32_t i, j, k;

    for (i = 1, j = 1, k = 0; i <= 16; )
    {
        if (j > BITS[i])
        {
            i++;
            j = 1;
        }
        else
        {
            HUFFSIZE[k++] = i;
            j++;
        }
    }
    HUFFSIZE[k] = 0;
    *lastk = k;
}

static void GenerateHuffCode(int32_t *HUFFSIZE, int32_t *HUFFCODE)
{
    for (int32_t k = 0, code = 0, si = HUFFSIZE[0]; ; )
    {
        HUFFCODE[k] = code;
        k++;
        code++;

        if (HUFFSIZE[k] == si)
        {
            continue;
        }
        else
        {
            if (HUFFSIZE[k] == 0)
            {
                break;
            }
            do {
                code = code << 1;
                si++;
            } while (HUFFSIZE[k] != si);
        }
    }
}

JpegCodec::HuffCodec::HuffCodec(const HuffTable &huffTable) :
    MINCODE{ 0 },
    MAXCODE{ 0 },
    VALPTR{ 0 }
{
    int32_t lastk;
    int32_t HUFFSIZE[HUFFVAL_SIZE] = { 0 };
    int32_t HUFFCODE[HUFFVAL_SIZE] = { 0 };

    GenerateHuffSize(huffTable.bits, HUFFSIZE, &lastk);
    GenerateHuffCode(HUFFSIZE, HUFFCODE);

    for (int32_t i = 0, j = 0; ; )
    {
        i++;
        if (i > 16)
        {
            break;
        }
        if (huffTable.bits[i] == 0)
        {
            MAXCODE[i] = -1;
        }
        else
        {
            VALPTR[i] = j;
            MINCODE[i] = HUFFCODE[j];
            j = (j + ((int32_t)huffTable.bits[i])) - 1;
            MAXCODE[i] = HUFFCODE[j];
            j++;
        }
    }
}

}
}
