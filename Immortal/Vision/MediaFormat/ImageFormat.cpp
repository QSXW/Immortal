#include "ImageFormat.h"
#include "Vision/Image/ImageCodec.h"
#include "FileSystem/FileSystem.h"

#include <cstdio>
#include <string>

namespace Immortal
{
namespace Vision
{

namespace
{

String FormatImageSequencePath(const String &pattern, int frameNumber)
{
    std::string text{ pattern.c_str(), pattern.size() };
    size_t placeholder = std::string::npos;
    size_t placeholderLength = 0;
    bool placeholderZeroPad = false;
    int placeholderWidth = 0;

    size_t percent = text.find('%');
    while (percent != std::string::npos)
    {
        size_t spec = percent + 1;
        bool zeroPad = false;
        int width = 0;
        if (spec < text.size() && text[spec] == '0')
        {
            zeroPad = true;
            ++spec;
        }
        while (spec < text.size() && text[spec] >= '0' && text[spec] <= '9')
        {
            width = width * 10 + (text[spec] - '0');
            ++spec;
        }
        if (spec < text.size() && text[spec] == 'd')
        {
            placeholder = percent;
            placeholderLength = spec - percent + 1;
            placeholderZeroPad = zeroPad;
            placeholderWidth = width;
        }
        percent = text.find('%', percent + 1);
    }
    if (placeholder != std::string::npos)
    {
        char number[64]{};
        if (placeholderZeroPad && placeholderWidth > 0)
        {
            std::snprintf(number, sizeof(number), "%0*d", placeholderWidth, frameNumber);
        }
        else
        {
            std::snprintf(number, sizeof(number), "%d", frameNumber);
        }
        text.replace(placeholder, placeholderLength, number);
        return String{ text, StringEncoding::UTF8 };
    }
    return pattern;
}

} // namespace

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
    String path = FormatImageSequencePath(filepath, frameNumber++);
    if (!path.empty())
	{
		Stream stream{path, StreamMode::Write};
		if (!stream.Writable())
		{
			CLOG_ERROR("Failed to open file {} to write the encoded image data!", path.c_str());
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
