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
        return new Vision::BMPCodec;

    case FileFormat::JPG:
    case FileFormat::JPEG:
    case FileFormat::JFIF:
#if HAVE_TURBOJPEG
		return new Vision::TurboJpegCodec;
#endif
    case FileFormat::HDR:
    case FileFormat::PNG:
        return new Vision::STBCodec;

    case FileFormat::PPM:
        return new Vision::PPMCodec;

    case FileFormat::ARW:
    case FileFormat::NEF:
    case FileFormat::CR2:
	case FileFormat::FFF:
	case FileFormat::RAF:
	case FileFormat::RW2:
        return new Vision::RawCodec{ Format::RGBA8 };

    default:
        return new Vision::OpenCVCodec;
        break;
    }
}

Picture Read(const std::string &path)
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

}
}
