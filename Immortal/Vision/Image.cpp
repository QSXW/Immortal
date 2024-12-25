#include "Image.h"
#include "Codec.h"
#include "Image/ImageCodec.h"
#include "FileSystem/FileSystem.h"

namespace Immortal
{
namespace Vision
{

static Codec *SelectSuitableCodec(const std::string &path)
{
    switch (FileSystem::DumpFileId(path))
    {
    case FileFormat::BMP:
        return new BMPCodec;

    case FileFormat::JPG:
    case FileFormat::JPEG:
    case FileFormat::JFIF:
#if HAVE_TURBOJPEG
		return new TurboJpegCodec;
#endif
    case FileFormat::HDR:
    case FileFormat::PNG:
        return new STBCodec;

#if HAVE_JXL:
	case FileFormat::JXL:
		return new JxlCodec;
#endif

    case FileFormat::PPM:
        return new PPMCodec;

    case FileFormat::ARW:
    case FileFormat::NEF:
    case FileFormat::CR2:
	case FileFormat::FFF:
	case FileFormat::RAF:
	case FileFormat::RW2:
        return new RawCodec{ Format::RGBA8 };

#if HAVE_WEBP
	case FileFormat::WEBP:
		return new WebpCodec{};
#endif
    default:
        return new OpenCVCodec;
        break;
    }
}

Picture Read(const String &path)
{
    Vision::CodedFrame codedFrame{ FileSystem::ReadBinary(path) };
    if (codedFrame.GetBuffer().empty())
    {
		return {};
    }

    URef<Interface::Codec> codec = SelectSuitableCodec(path);
    if (codec->Decode(codedFrame) != CodecError::Success)
    {
		return {};
    }

    return codec->GetPicture();
}

CodedFrame Write(const Picture &picture, const String &path)
{
	URef<Interface::Codec> codec = SelectSuitableCodec(path);

	CodedFrame codedFrame;
	if (codec->Encode(picture, codedFrame) != CodecError::Success)
	{
		return {};
	}
    
    if (!path.empty())
    {
		Stream stream{path, StreamMode::Write};
        if (!stream.Writable())
        {
			LOG::ERR("Failed to open file {} to write the encoded image data!", path.c_str());
			return {};
        }
		stream.Write(codedFrame.GetBuffer());
    }

	return codedFrame;
}

}
}
