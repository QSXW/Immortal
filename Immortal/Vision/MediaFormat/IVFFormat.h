#pragma once

#include "Codec.h"
#include "MediaFormat.h"
#include "FileSystem/Stream.h"

namespace Immortal
{
namespace Vision
{

class IMMORTAL_API IVFFormat : public MediaFormat
{
public:
    struct Header
    {
        size_t   size;
        uint64_t offset;
        uint64_t timestamp;
    };

public:
	IVFFormat();

    virtual CodecError Open(const String &filepath) override;

    virtual CodecError Read(CodedFrame *codedFrame) override;

private:
    Header ReadHeader();

protected:
    Stream stream;
};

}
}
