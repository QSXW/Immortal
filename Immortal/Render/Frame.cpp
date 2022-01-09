#include "impch.h"
#include "Frame.h"

#include "Texture.h"
#include "Media/StillPicture.h"
#include "FileSystem/FileSystem.h"

namespace Immortal
{

Frame::Frame(const std::string &path)
{
    if (FileSystem::IsFormat<FileFormat::BMP>(path))
    {
        auto bmpDecoder = new Media::BMPDecoder{};
        bmpDecoder->Read(path);
        decoder.reset(bmpDecoder);
    }
    else
    {
        auto buf = FileSystem::ReadBinary(path);
        if (buf.empty())
        {
            return;
        }

        decoder.reset(new Media::OpenCVCodec());
        if (decoder->Decode(buf) != Media::CodecError::Succeed)
        {
            decoder.reset(new Media::STBCodec());
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
