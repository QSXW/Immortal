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
    virtual ~OpenCVCodec();

    virtual CodecError Decode(const std::vector<uint8_t> &buf) override;

    virtual uint8_t *Data() const;

    virtual void Swap(void *ptr);

private:
    cv::Mat mat;
#endif
};

}
}
