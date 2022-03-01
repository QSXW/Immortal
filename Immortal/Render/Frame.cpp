#include "impch.h"
#include "Frame.h"

#include "Texture.h"
#include "Media/Image/Image.h"
#include "FileSystem/FileSystem.h"

namespace Immortal
{

Frame::Frame(const std::string &path)
{
    if (FileSystem::IsFormat<FileFormat::BMP>(path))
    {
        auto bmpCodec = new Vision::BMPCodec{};
        bmpCodec->Read(path);
        decoder.reset(bmpCodec);
    }
    else
    {
        auto buf = FileSystem::ReadBinary(path);
        if (buf.empty())
        {
            return;
        }

        decoder.reset(new Vision::STBCodec());
        if (decoder->Decode(buf) != CodecError::Succeed)
        {
            decoder.reset(new Vision::OpenCVCodec());
            {
                decoder->Decode(buf);
            }
        }
    }
}

Frame::Frame(uint32_t width, uint32_t height, int depth, const void *data)
{

}

Frame::~Frame()
{

}

}
