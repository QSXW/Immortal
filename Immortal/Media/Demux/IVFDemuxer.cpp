#include "IVFDemuxer.h"
#include "Stream.h"
#include "Math/Math.h"

namespace Immortal
{
namespace Vision
{

struct DoubleWord
{
    DoubleWord(const uint8_t *const p)
    {
        v = ((uint32_t)p[3] << 24U) | (p[2] << 16U) | (p[1] << 8U) | p[0];
    }

    operator uint32_t() const
    {
        return v;
    }

    uint32_t v;
};

struct QuardWord
{
    QuardWord(const uint8_t *const p)
    {
        v = (((uint64_t)DoubleWord(&p[4])) << 32) | DoubleWord(p);
    }

    operator uint64_t() const
    {
        return v;
    }

    uint64_t v;
};

IVFDemuxer::IVFDemuxer() :
    stream{ Stream::Mode::Read }
{

}

CodecError IVFDemuxer::Open(const std::string &filepath, VideoCodec *codec)
{
    uint8_t data[32];
    auto animator = codec->GetAddress<Animator>();

    stream.Open(filepath);
    if (!stream.Readable())
    {
        return CodecError::FailedToOpenFile;
    }
    if (stream.Read(data, 32, 1) != 1) {
        LOG::ERR("Failed to read stream data: {}\n", strerror(errno));
        return CodecError::CorruptedBitstream;
    }
    else if (memcmp(data, "DKIF", 4) || memcmp(&data[8], "AV01", 4)) {
        LOG::ERR("{} is not an IVF file\n", filepath.c_str());
        return CodecError::CorruptedBitstream;
    }

    Rational timebase{ 
        (uint32_t)DoubleWord{ &data[16] },
        (uint32_t)DoubleWord{ &data[20] }
    };
    animator->timebase = timebase.Normalize();

    uint32_t duration = DoubleWord{ &data[24] };
    uint32_t lastTimestamp = 0;
    for (animator->frameNumber = 0; ; animator->frameNumber++)
    {
        if (stream.Read(data, 4, 1) != 1)
        {
            break;
        }
        uint32_t size = DoubleWord{ data };
        
        if (stream.Read(data, 8, 1) != 1)
        {
            break;
        }
        uint64_t timestamp = QuardWord{ data };
        if (animator->frameNumber && timestamp <= lastTimestamp)
        {
            return CodecError::CorruptedBitstream;
        }
        lastTimestamp = timestamp;
        stream.Skip(size);
    }

    Rational fps{
        timebase.numerator * animator->frameNumber,
        duration
    };

    animator->step = duration / animator->frameNumber;

    stream.Locate(32);
    return CodecError::Succeed;
}

CodecError IVFDemuxer::Read(CodedFrame *codedFrame)
{
    auto header = ReadHeader();
    codedFrame->buffer.resize(header.size);
    ThrowIf(
        stream.Read(codedFrame->buffer.data(), codedFrame->buffer.size()) != 1,
        "Failed to coded read frame data"
    );

    codedFrame->offset = header.offset;
    codedFrame->timestamp = header.timestamp;

    return CodecError::Succeed;
}

IVFDemuxer::Header IVFDemuxer::ReadHeader()
{
    Header header{};

    uint8_t data[8];
    header.offset = stream.Pos();
    if (stream.Read(data, 4, 1) == 1)
    {
        header.size = DoubleWord{ data };
    }
    if (stream.Read(data, 8, 1) == 1)
    {
        header.timestamp = QuardWord{ data };
    }

    return header;
}

}
}
