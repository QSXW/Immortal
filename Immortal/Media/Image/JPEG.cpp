#include "JPEG.h"
#include "Media/LookupTable/LookupTable.h"
#include "slintrinsic.h"

namespace Immortal
{
namespace Vision
{

static inline void Dequantize(int16_t *block, int16_t *table)
{
    int16x32 b1{ block };
    int16x32 b2{ block + 32 };

    int16x32 t1{ table };
    int16x32 t2{ table + 32 };

    b1 = b1 * t1;
    b2 = b2 * t2;

    b1.aligned_store(block);
    b2.aligned_store(block + 32);
}

static const double A[] = {
    NAN,
    0.707106781186547524400844,
    0.541196100146196984399723,
    0.707106781186547524400844,
    1.306562964876376527856643,
    0.382683432365089771728460,
};

static const double S[] = {
    0.353553390593273762200422,
    0.254897789552079584470970,
0.270598050073098492199862,
0.300672443467522640271861,
0.353553390593273762200422,
0.449988111568207852319255,
0.653281482438188263928322,
1.281457723870753089398043,
};

#define IDCT8(C, stride) \
	v15 = C[stride * 0] / S[0]; \
    v26 = C[stride * 1] / S[1]; \
    v21 = C[stride * 2] / S[2]; \
    v28 = C[stride * 3] / S[3]; \
	v16 = C[stride * 4] / S[4]; \
    v25 = C[stride * 5] / S[5]; \
    v22 = C[stride * 6] / S[6]; \
    v27 = C[stride * 7] / S[7]; \
	v19 = (v25 - v28) / 2; \
    v20 = (v26 - v27) / 2; \
    v23 = (v26 + v27) / 2; \
    v24 = (v25 + v28) / 2; \
	v7  = (v23 + v24) / 2; \
    v11 = (v21 + v22) / 2; \
    v13 = (v23 - v24) / 2; \
    v17 = (v21 - v22) / 2; \
	v8  = (v15 + v16) / 2; \
    v9  = (v15 - v16) / 2; \
	v18 = (v19 - v20) * A[5]; \
	v12 = (v19 * A[4] - v18) / (A[2] * A[5] - A[2] * A[4] - A[4] * A[5]); \
	v14 = (v18 - v20 * A[2]) / (A[2] * A[5] - A[2] * A[4] - A[4] * A[5]); \
	v6 = v14 - v7; \
	v5 = v13 / A[3] - v6; \
	v4 = -v5 - v12; \
	v10 = v17 / A[1] - v11; \
	v0 = (v8 + v11) / 2; \
	v1 = (v9 + v10) / 2; \
	v2 = (v9 - v10) / 2; \
	v3 = (v8 - v11) / 2; \
	C[stride * 0] = (v0 + v7) / 2; \
	C[stride * 1] = (v1 + v6) / 2; \
	C[stride * 2] = (v2 + v5) / 2; \
	C[stride * 3] = (v3 + v4) / 2; \
	C[stride * 4] = (v3 - v4) / 2; \
	C[stride * 5] = (v2 - v5) / 2; \
	C[stride * 6] = (v1 - v6) / 2; \
	C[stride * 7] = (v0 - v7) / 2;

#define IDCT8_H(C, O) IDCT8((C + O), 1)
#define IDCT8_V(C, O) IDCT8((C + O), 8)

template <class T>
inline constexpr void InverseDCT8x8(T *block)
{
    double  v0,  v1,  v2,  v3;
    double  v4,  v5,  v6,  v7;
    double  v8,  v9, v10, v11;
    double v12, v13, v14, v15;
    double v16, v17, v18, v19;
    double v20, v21, v22, v23;
    double v24, v25, v26, v27, v28;

    IDCT8_H(block, 0 );
    IDCT8_H(block, 8 );
    IDCT8_H(block, 16);
    IDCT8_H(block, 24);
    IDCT8_H(block, 32);
    IDCT8_H(block, 48);
    IDCT8_H(block, 56);

    IDCT8_V(block, 0);
    IDCT8_V(block, 1);
    IDCT8_V(block, 2);
    IDCT8_V(block, 3);
    IDCT8_V(block, 4);
    IDCT8_V(block, 5);
    IDCT8_V(block, 6);
    IDCT8_V(block, 7);
}

static inline void Levelup(int16_t *block)
{
    int16x32 b1{ block };
    int16x32 b2{ block + 32 };

    int16x32 level{ int16_t(128) };
    b1 = b1 + level;
    b2 = b2 + level;

    b1.convert<uint8x32>().aligned_store(block);
    b2.convert<uint8x32>().aligned_store(block + 16);
}

#define COPY_64BITS(n) *(uint64_t *)(dst + n * stride) = *(((uint64_t *)block) + n)
static void Backward(uint8_t *dst, size_t stride, int16_t *block, int16_t *table)
{
    Dequantize(block, table);
    InverseDCT8x8(block);
    Levelup(block);

    COPY_64BITS(0);
    COPY_64BITS(1);
    COPY_64BITS(2);
    COPY_64BITS(3);
    COPY_64BITS(4);
    COPY_64BITS(5);
    COPY_64BITS(6);
    COPY_64BITS(7);
}

#define HuffReceive(s) bitTracker.GetBits(s)
static inline int32_t HuffExtend(int32_t r, int32_t s)
{
    return LookupTable::ExponentialGolobm[s][r];
}

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
        break;
    }

    ThrowIf(true, "Incorrect Samping Factor");
    return Format::None;
}

static inline int32_t AlignToMCU(int32_t v, int32_t samplingFactor)
{
    return SLALIGN(v, JpegCodec::BLOCK_WIDTH * samplingFactor);
}

JpegCodec::JpegCodec(const std::vector<uint8_t> &buffer)
{
    ParseHeader(buffer);
}

JpegCodec::~JpegCodec()
{
    if (buffer)
    {
        allocator.deallocate(buffer, BLOCK_SIZE);
    }
    if (data.x)
    {
        allocator.deallocate(data.x, BLOCK_SIZE);
    }
}

void JpegCodec::ParseHeader(const std::vector<uint8_t> &buffer)
{
    ThrowIf(buffer[0] != 0xff && buffer[1] != 0xd8, "Not a Jpeg file");

    auto *end = buffer.data() + buffer.size();
    for (auto ptr = &buffer[2]; ptr < end; )
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
            case MarkerType::APP2:
            case MarkerType::APP3:
            case MarkerType::APP4:
            case MarkerType::APP5:
            case MarkerType::APP6:
            case MarkerType::APP7:
            case MarkerType::APP8:
            case MarkerType::APP9:
            case MarkerType::APPA:
            case MarkerType::APPB:
            case MarkerType::APPC:
            case MarkerType::APPD:
            case MarkerType::APPE:
                ParseMarker(&ptr, [&](auto payload) { JpegCodec::ParseAPP(payload); });
                break;

            case MarkerType::DRI:
                ParseMarker(&ptr, [&](auto payload) { JpegCodec::ParseDRI(payload); });
                break;

            default:
                LOG::WARN("Unsupport Jpeg Marker Type: {}", *ptr++);
                break;
            }
        }
        else
        {
            ptr++;
        }
    }
}

CodecError JpegCodec::Decode()
{
    InitDecodedPlaneBuffer();
    if (!isProgressive)
    {
        DecodeMCU();
    }
    ConvertColorSpace();
    return CodecError::Succeed;
}

uint8_t *JpegCodec::Data() const
{
    return data.x;
}

inline void JpegCodec::ParseAPP(const uint8_t *data)
{
#ifndef JPEG_CODEC_MINIMAL
    uint8_t type = data[-3];

    if (type > MarkerType::APP0 && type <= MarkerType::APPF)
    {
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
    uint16_t length = Word{ &data[-2] } - 2;
    size_t i = 0;

    do
    {
        uint8_t selector = data[i++];
        uint8_t precision = (selector & 0xf0) >> 4;
        auto &table = quantizationTables[selector & 0x0f];
        for (size_t j = 0; j < table.size(); j++)
        {
            if (precision)
            {
                table[LookupTable::ZigZagToNaturalOrder[j]] = ((int16_t *)data)[i];
                i += 2;
            }
            else
            {
                table[LookupTable::ZigZagToNaturalOrder[j]] = data[i++];
            }
        }
    } while (i < length);
}

inline void JpegCodec::ParseDHT(const uint8_t *data)
{
    uint16_t length = Word{ &data[-2] } - 2;
    size_t i = 0;

    do
    {
        uint8_t selector = data[i++];
        uint8_t index = selector & 0x0f;
        auto &table = (0x10 & selector) ? actbl[index] : dctbl[index];
        CleanUpObject(&table);

        size_t tableLength = 0;
        int j = 1;
        while (j < SL_ARRAY_LENGTH(table.bits))
        {
            tableLength += table.bits[j++] = data[i++];
        }
        memcpy(table.huffval, &data[i], tableLength);
        i += tableLength;

        table.Init();
    } while (i < length);
}

inline void JpegCodec::ParseDRI(const uint8_t *data)
{
    restartInterval = data[0];
}

inline void JpegCodec::ParseSOF(const uint8_t *data)
{
    if (isProgressive)
    {
        return;
    }

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

        component.x = AlignToMCU(component.width, component.sampingFactor.horizontal);
        component.y = AlignToMCU(component.height, component.sampingFactor.vertical);

        blocksInMCU += component.sampingFactor.horizontal * component.sampingFactor.vertical;
    }
    mcu.x = components[0].x / (8 * components[0].sampingFactor.horizontal);
    mcu.y = components[0].y / (8 * components[0].sampingFactor.vertical);
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
        auto selector = ptr[1];
        components[index].acIndex = (selector     ) & 0xf;
        components[index].dcIndex = (selector >> 4) & 0xf;
    }
}

void JpegCodec::InitDecodedPlaneBuffer()
{
    size_t size = 0;

    auto planes = desc.format.ComponentCount();
    std::array<uint32_t, 4> offsets{ 0 };
    for (size_t i = 0; i < planes; i++)
    {
        offsets[i] = size;
        size += SLALIGN(components[i].x * components[i].y, BLOCK_SIZE);
    }
    buffer = allocator.allocate(size);

    size_t blockIndex = 0;
    for (size_t i = 0; i < planes; i++)
    {
        uint32_t offset = offsets[i];
        uint32_t stride = components[i].x * 8;
        for (size_t v = 0; v < components[i].sampingFactor.vertical; v++)
        {
            for (size_t h = 0; h < components[i].sampingFactor.horizontal; h++)
            {
                auto &block = blocks[blockIndex];
                block.componentIndex = i;
                block.offset = components[i].sampingFactor.horizontal * BLOCK_WIDTH;
                block.stride = stride * components[i].sampingFactor.vertical;
                block.data = &buffer[offset + h * BLOCK_WIDTH + v * stride];
                blockIndex++;
            }
        }
    }
}

void JpegCodec::DecodeMCU()
{
    int32_t SL_ALIGNED(16) pred[4] = { 0 };
    auto dstBlocks = blocks;
    for (size_t y = 0; y < mcu.y; y++)
    {
        for (size_t x = 0; x < mcu.x; x++)
        {
            for (size_t i = 0; i < blocksInMCU; i++)
            {
                int16_t blockBuffer[BLOCK_SIZE] = { 0 };
                auto &block = dstBlocks[i];
                auto index = block.componentIndex;
                auto &component = components[index];
                DecodeBlock(blockBuffer, dctbl[component.dcIndex], actbl[component.acIndex], &pred[index]);
                Backward(&block.data[y * block.stride + x * block.offset], component.x, blockBuffer, quantizationTables[component.qtSelector].data());
            }
            if (bitTracker.RestartMarker)
            {
                int32x4 zero{ 0 };
                zero.aligned_store(pred);
                bitTracker.RestartMarker = false;
                break;
            }
        }
    }
}

void JpegCodec::DecodeBlock(int16_t *block, HuffTable &dcTable, HuffTable &acTable, int32_t *pred)
{
    int32_t s;
    int32_t r;

    /* DC Coefficient */
    s = HuffDecode(dcTable);
    if (s)
    {
        r = HuffReceive(s);
        s = HuffExtend(r, s);
    }
    *pred += s;
    block[0] = *pred;

    /* AC Coefficients */
    for (size_t k = 1; k < 64; )
    {
        s = HuffDecode(acTable);
        r = s >> 4;
        s &= 0xf;

        if (s)
        {
            k += r;
            r = HuffReceive(s);
            block[LookupTable::ZigZagToNaturalOrder[k++]] = HuffExtend(r, s);
        }
        else
        {
            if (r != 0xf)
            {
                break;
            }
            k += 0x10;
        }
    }
}

int32_t JpegCodec::HuffDecode(HuffTable &huffTable)
{
    int64_t code = 0;
    for (int32_t i = 1, j = 0; ; )
    {
        code = (code << 1) + bitTracker.GetBits(1);
        if (code > huffTable.MAXCODE[i])
        {
            i++;
        }
        else
        {
            j = huffTable.VALPTR[i];
            j = j + code - huffTable.MINCODE[i];
            return huffTable.huffval[j];
        }
    }
}

void JpegCodec::ConvertColorSpace()
{
    auto spatial = desc.Spatial();
    ColorSpace::Vector<uint8_t> yuv;
    data.x = allocator.allocate(spatial * Format{ Format::RGBA8 }.ComponentCount());

    for (size_t i = 0, offset = 0; i < components.size(); i++)
    {
        yuv[i] = buffer + offset;
        offset += SLALIGN(components[i].x * components[i].y, BLOCK_SIZE);
    }

    if (desc.format == Format::YUV444P)
    {
        ColorSpace::YUV444PToRGBA8(data, yuv, desc.width, desc.height, SLALIGN(desc.width, 8));
    }
    else if (desc.format == Format::YUV420P)
    {
        ColorSpace::YUV420PToRGBA8(data, yuv, desc.width, desc.height, SLALIGN(desc.width, 8), SLALIGN(desc.width, 8) / 2);
    }
    desc.format = Format::RGBA8;
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

void JpegCodec::HuffTable::Init()
{
    int32_t lastk;
    int32_t HUFFSIZE[HUFFVAL_SIZE] = { 0 };
    int32_t HUFFCODE[HUFFVAL_SIZE] = { 0 };

    GenerateHuffSize(bits, HUFFSIZE, &lastk);
    GenerateHuffCode(HUFFSIZE, HUFFCODE);

    for (int32_t i = 0, j = 0; ; )
    {
        i++;
        if (i > 16)
        {
            break;
        }
        if (bits[i] == 0)
        {
            MAXCODE[i] = -1;
        }
        else
        {
            VALPTR[i] = j;
            MINCODE[i] = HUFFCODE[j];
            j = (j + ((int32_t)bits[i])) - 1;
            MAXCODE[i] = HUFFCODE[j];
            j++;
        }
    }
}

}
}
