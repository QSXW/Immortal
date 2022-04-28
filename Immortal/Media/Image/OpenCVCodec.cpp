#include "OpenCVCodec.h"

namespace Immortal
{
namespace Vision
{

#if HAVE_OPENCV
OpenCVCodec::~OpenCVCodec()
{
        
}

CodecError OpenCVCodec::Decode(const CodedFrame &codedFrame)
{
    const auto &buf = codedFrame.buffer;
    cv::Mat	src = cv::imdecode(cv::Mat{ buf }, cv::IMREAD_UNCHANGED);
    if (!src.data)
    {
        LOG::ERR("Failed to load image -> {{0}}", buf.data());
        return CodecError::CorruptedBitstream;
    }
    if (src.data)
    {
        if (src.channels() == 4)
        {
            cv::cvtColor(src, mat, cv::COLOR_BGRA2RGBA);
        }
        else
        {
            cv::cvtColor(src, mat, cv::COLOR_BGR2RGBA);
        }
    }

    desc.width  = mat.cols;
    desc.height = mat.rows;
    desc.format = Format::RGBA8;

    if (mat.depth() == CV_16U)
    {
        desc.format = Format::RGBA16;
    }
    if (mat.depth() == CV_32FC4)
    {
        desc.format = Format::RGBA32F;
    }

    return CodecError::Succeed;
}

uint8_t *OpenCVCodec::Data() const
{
    return mat.data;
}

void OpenCVCodec::Swap(void *ptr)
{
    ptr = new uint8_t[desc.Size()];
    if (!ptr)
    {
        return;
    }
    memcpy(ptr, mat.data, desc.Size());
}
#endif

}
}
