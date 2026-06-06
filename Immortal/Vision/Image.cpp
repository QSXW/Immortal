#include "Image.h"
#include "Codec.h"
#include "Image/ImageCodec.h"
#include "FileSystem/FileSystem.h"

namespace Immortal
{
namespace Vision
{

Codec *SelectSuitableCodec(const std::string &path, bool decoder, const ImageEncodeInfo &info)
{
    switch (FileSystem::DumpFileId(path))
    {
    case FileFormat::BMP:
        return new BMPCodec;

    case FileFormat::JPG:
    case FileFormat::JPEG:
    case FileFormat::JFIF:
#if HAVE_TURBOJPEG
		return new TurboJpegCodec{info};
#endif
    case FileFormat::PNG:
#if HAVE_PNG
		return new PNGCodec{info};
#endif
    case FileFormat::HDR:
        return new STBCodec;

    case FileFormat::JXL:
#if HAVE_JXL:
		return new JxlCodec;
#endif
		return new FFJpegxlCodec;

    case FileFormat::TIFF:
		return new FFTiffCodec;

    case FileFormat::PPM:
        return new PPMCodec;

    case FileFormat::ARW:
    case FileFormat::NEF:
    case FileFormat::CR2:
	case FileFormat::FFF:
	case FileFormat::RAF:
	case FileFormat::RW2:
        return new RawCodec{ Format::RGBA8 };

    case FileFormat::WEBP:
#if HAVE_WEBP
		return new WebpCodec{};
#endif
		return new FFWebpCodec;

    default:
        return new OpenCVCodec;
        break;
    }
}

Picture Read(const String &path)
{
    Vision::CodedFrame codedFrame{ FileSystem::ReadBinary(path) };
    if (!codedFrame.GetData())
    {
		return {};
    }

    URef<Interface::Codec> codec = SelectSuitableCodec(path, true);
    if (!codec)
    {
		return {};
    }

    if (codec->Decode(codedFrame) != CodecError::Success)
    {
		return {};
    }

    return codec->GetPicture();
}

CodedFrame Write(const Picture &picture, const String &path, const ImageEncodeInfo &info)
{
	URef<Interface::Codec> codec = SelectSuitableCodec(path, false, info);

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
		stream.Write(codedFrame.GetData(), codedFrame.GetSize());
    }

	return codedFrame;
}

}
}
