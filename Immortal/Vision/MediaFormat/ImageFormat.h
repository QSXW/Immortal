#pragma once

#include "Config.h"
#include "Vision/Codec.h"
#include "Vision/MediaFormat.h"
#include "FileSystem/Stream.h"
#include "String/IString.h"
#include <vector>
#include <memory>

namespace Immortal
{
namespace Vision
{

class ImageFormat : public MediaFormat
{
public:
public:
    ImageFormat();

    ~ImageFormat();

    virtual CodecError Open(const String &filepath) override;

    virtual CodecError Open(const String &filepath, Codec **pCodec, const CodecInfo *encodeInfos, uint32_t numCodec) override;

    virtual void Close() override;

    virtual CodecError Read(CodedFrame *codedFrame) override;

    virtual CodecError Write(const CodedFrame &codedFrame, int stream = 0) override;

    virtual CodecError Seek(MediaType type, int64_t pts, int64_t min, int64_t max) override;

    virtual const String &GetSource() const override;

    void Destroy();

private:
    String filepath;

    int frameNumber;

    bool isOpen;
};

}
}
