#include "OpenCVCodec.h"

#if HAVE_OPENCV
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp> 
#include <opencv2/imgcodecs/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#endif

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

    picture.Reset(mat.data);
    mat.data = nullptr;

    return CodecError::Succeed;
}

CodecError OpenCVCodec::Encode(const Picture &picture, CodedFrame &codedFrame)
{
    cv::Mat input{ cv::Size{ (int)picture.desc.width, (int)picture.desc.height }, CV_8UC4, picture.Data()};
    try
    {
        if (!cv::imencode(".jpg", input, codedFrame.buffer))
        {
            return CodecError::FailedToCallDecoder;
        }
    }
    catch (const std::exception &e)
    {
        LOG::ERR("{}", e.what());
    }

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
    memcpy(ptr, picture.Data(), size);
}
#endif

}
}
