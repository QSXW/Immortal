#pragma once

#include <cstdint>
#include "Decoder.h"

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp> 
#include <opencv2/imgcodecs/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

namespace Immortal
{
namespace Media
{

class OpenCVCodec : public Decoder
{
public:
    OpenCVCodec()
    {
        
    }

    virtual ~OpenCVCodec()
    {
        
    }

    virtual CodecError Decode(const std::vector<uint8_t> &buf)
    {
        cv::Mat	src = cv::imdecode(cv::Mat{ buf }, cv::IMREAD_UNCHANGED);
        if (!src.data)
        {
            LOG::ERR("Failed to load image -> {{0}}", buf.data());
            return CodecError::CorruptedBitstream;
        }
        if (src.data)
        {
            desc.Depth = src.channels();
            if (desc.Depth == 4)
            {
                cv::cvtColor(src, mat, cv::COLOR_BGRA2RGBA);
            }
            else
            {
                cv::cvtColor(src, mat, cv::COLOR_BGR2RGBA);
            }
        }

        desc.Width  = mat.cols;
        desc.Height = mat.rows;
        desc.Depth  = mat.channels();
        desc.Format = Format::RGBA8;

        if (mat.depth() == CV_16U)
        {
            desc.Format = Format::RGBA16;
        }
        if (mat.depth() == CV_32FC4)
        {
            desc.Format = Format::RGBA32F;
        }

        FillUpDescription();
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
        ptr = new uint8_t[desc.Size];
        if (!ptr)
        {
            return;
        }
        memcpy(ptr, mat.data, desc.Size);
    }

private:
    cv::Mat mat;
};

}
}
