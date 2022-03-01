#pragma once

#include <cstdint>

#include "Media/Interface/Codec.h"
#include "Config.h"

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

class OpenCVCodec : public Interface::Codec
{
public:
    using Super = Interface::Codec;

public:
    OpenCVCodec()
    {
        
    }

#if HAVE_OPENCV
    virtual ~OpenCVCodec()
    {
        
    }

    virtual CodecError Decode(const std::vector<uint8_t> &buf) override
    {
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

    virtual void Encode() 
    {

    }

    virtual uint8_t *Data() const
    {
        return mat.data;
    }

    virtual void Swap(void *ptr)
    {
        ptr = new uint8_t[desc.Size()];
        if (!ptr)
        {
            return;
        }
        memcpy(ptr, mat.data, desc.Size());
    }

private:
    cv::Mat mat;
#endif
};

}
}
