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
    cv::Mat mat;

    auto &desc = picture.desc;
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

    picture.data.reset(mat.data);
    mat.data = nullptr;

    return CodecError::Succeed;
}

void OpenCVCodec::Swap(void *ptr)
{
    size_t size = picture.desc.Size();
    ptr = new uint8_t[size];
    if (!ptr)
    {
        return;
    }
    memcpy(ptr, picture.data.get(), size);
}
#endif

}
}
