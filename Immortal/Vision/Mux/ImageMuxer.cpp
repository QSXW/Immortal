#include "ImageMuxer.h"
#include "Vision/Image/ImageCodec.h"
#include "FileSystem/FileSystem.h"

namespace Immortal
{
namespace Vision
{

ImageMuxer::ImageMuxer() :
    frameNumber{1}
{

}

ImageMuxer::~ImageMuxer()
{
    Close();
}

CodecError ImageMuxer::Open(const String &_filepath, VideoCodec *codec, VideoCodec *audioCodec, VideoCodec *subtitleCodec)
{
    (void)audioCodec;
    (void)subtitleCodec;
    
    return CodecError::Success;
}

CodecError ImageMuxer::Open(const String &filepath, Codec **pCodec, const EncodeInfo *encodeInfos, uint32_t numCodec)
{
    (void)pCodec;
    (void)numCodec;
    
    this->filepath = filepath;

    return CodecError::Success;
}

void ImageMuxer::Close()
{
    isOpen = false;
}

CodecError ImageMuxer::Read(CodedFrame *codedFrame)
{
    (void)codedFrame;
    return CodecError::NotImplement;
}

CodecError ImageMuxer::Write(const CodedFrame &codedFrame, int stream)
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

CodecError ImageMuxer::Seek(MediaType type, double seconds, int64_t min, int64_t max)
{
    return CodecError::NotImplement;
}

void ImageMuxer::Destroy()
{
    Close();
}

const String &ImageMuxer::GetSource() const
{
    return filepath;
}

}
}
