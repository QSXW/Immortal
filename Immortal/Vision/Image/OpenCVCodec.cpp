#include "OpenCVCodec.h"

#if HAVE_OPENCV
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <utility>
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

    const uint8_t *buf  = codedFrame.GetData();
	const size_t   size = codedFrame.GetSize();
	cv::Mat src = cv::imdecode(cv::InputArray{buf, (int)size}, cv::IMREAD_UNCHANGED);
    if (!src.data)
    {
        return CodecError::CorruptedBitstream;
    }
    switch (src.channels())
    {
    case 4:
        cv::cvtColor(src, mat, cv::COLOR_BGRA2RGBA);
        break;
    case 3:
        cv::cvtColor(src, mat, cv::COLOR_BGR2RGBA);
        break;
    case 1:
        cv::cvtColor(src, mat, cv::COLOR_GRAY2RGBA);
        break;
    default:
        return CodecError::CorruptedBitstream;
    }
    if (mat.empty())
    {
        return CodecError::CorruptedBitstream;
    }

    Format format = Format::RGBA8;
    if (mat.depth() == CV_16U)
    {
        format = Format::RGBA16;
    }
    else if (mat.depth() == CV_32F)
    {
        format = Format::R32G32B32A32_SFLOAT;
    }

    auto ownedMat = new cv::Mat{ std::move(mat) };
    picture = Picture{ ownedMat->cols, ownedMat->rows, format };
    picture.SetData(ownedMat->data);
    picture.SetStride(0, static_cast<uint32_t>(ownedMat->step[0]));
    picture.SetRelease([ownedMat] (void *) {
        delete ownedMat;
    });

    return CodecError::Success;
}

CodecError OpenCVCodec::Encode(const Picture &picture, CodedFrame &codedFrame)
{
    cv::Mat input{ cv::Size{ (int)picture.GetWidth(), (int)picture.GetHeight() }, CV_8UC4, picture.GetData()};
    try
    {
		std::vector<uint8_t> buffer;
		if (!cv::imencode(".jpg", input, buffer))
        {
            return CodecError::FailedToCallDecoder;
        }

        codedFrame = { std::move(buffer) };
    }
    catch (const std::exception &e)
    {
        LOG::ERR("{}", e.what());
    }

    return CodecError::Success;
}

void OpenCVCodec::Swap(void *ptr)
{
	size_t size = picture.GetWidth() * picture.GetHeight() * picture.GetFormat().GetTexelSize();
    ptr = new uint8_t[size];
    if (!ptr)
    {
        return;
    }
    memcpy(ptr, picture.GetData(), size);
}
#endif

}
}
