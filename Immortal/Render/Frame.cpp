#include "Frame.h"

#include "Texture.h"
#include "Vision/Image/Image.h"
#include "FileSystem/FileSystem.h"

namespace Immortal
{

static Vision::Interface::Codec *SelectSuitableCodec(const std::string &path)
{
    switch (FileSystem::DumpFileId(path))
    {
    case FileFormat::BMP:
        return new Vision::BMPCodec{};

    case FileFormat::JPG:
    case FileFormat::JPEG:
    case FileFormat::JFIF:
        return new Vision::MFXJpegCodec{};

    case FileFormat::HDR:
    case FileFormat::PNG:
        return new Vision::STBCodec{};

    case FileFormat::PPM:
        return new Vision::PPMCodec{};

    default:
        return new Vision::OpenCVCodec{};
        break;
    }
}

Frame::Frame(const std::string &path)
{
    Vision::CodedFrame codedFrame{};
    codedFrame.Assign(FileSystem::ReadBinary(path));

    decoder.Reset(SelectSuitableCodec(path));
    decoder->Decode(codedFrame);
}

Frame::Frame(uint32_t width, uint32_t height, int depth, const void *data)
{

}

Frame::~Frame()
{

}

}
