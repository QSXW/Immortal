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
        auto bmpCodec = new Media::BMPCodec{};
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

        decoder.reset(new Media::STBCodec());
        if (decoder->Decode(buf) != Media::CodecError::Succeed)
        {
            decoder.reset(new Media::OpenCVCodec());
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
