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
    case FileFormat::HDR:
    case FileFormat::PNG:
        return new Vision::STBCodec;

    case FileFormat::PPM:
        return new Vision::PPMCodec;

    case FileFormat::ARW:
    case FileFormat::NEF:
    case FileFormat::CR2:
        return new Vision::RawCodec;

    default:
        return new Vision::OpenCVCodec;
        break;
    }
}

Picture Read(const std::string &path)
{
    Vision::CodedFrame codedFrame{};
    codedFrame.Assign(FileSystem::ReadBinary(path));

    URef<Interface::Codec> codec = SelectSuitableCodec(path);
    codec->Decode(codedFrame);

    return codec->GetPicture();
}

}
}
