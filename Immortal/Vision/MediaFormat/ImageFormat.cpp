#include "ImageFormat.h"
#include "Vision/Image/ImageCodec.h"
#include "FileSystem/FileSystem.h"

namespace Immortal
{
namespace Vision
{

ImageFormat::ImageFormat() :
    frameNumber{1}
{

}

ImageFormat::~ImageFormat()
{
    Close();
}

CodecError ImageFormat::Open(const String &filepath)
{
	this->filepath = filepath;

	return CodecError::Success;
}

CodecError ImageFormat::Open(const String &filepath, Codec **pCodec, const CodecInfo *encodeInfos, uint32_t numCodec)
{
    (void)pCodec;
    (void)numCodec;

    this->filepath = filepath;

    return CodecError::Success;
}

void ImageFormat::Close()
{
    isOpen = false;
}

CodecError ImageFormat::Read(CodedFrame *codedFrame)
{
    (void)codedFrame;
    return CodecError::NotImplement;
}

CodecError ImageFormat::Write(const CodedFrame &codedFrame, int stream)
{
	char data[4096] = {};
	sprintf(data, filepath.c_str(), frameNumber++);

    std::filesystem::path path = data;
    if (!path.empty())
	{
		Stream stream{path, StreamMode::Write};
		if (!stream.Writable())
		{
			CLOG_ERROR("Failed to open file {} to write the encoded image data!", path.string());
			return CodecError::ExternalFailed;
		}
		stream.Write(codedFrame.GetData(), codedFrame.GetSize());
	}

    return CodecError::Success;
}

CodecError ImageFormat::Seek(MediaType type, int64_t pts, int64_t min, int64_t max)
{
    return CodecError::NotImplement;
}

void ImageFormat::Destroy()
{
    Close();
}

const String &ImageFormat::GetSource() const
{
    return filepath;
}

}
}
