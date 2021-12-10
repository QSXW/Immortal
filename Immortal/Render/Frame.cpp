#include "impch.h"
#include "Frame.h"

#include "Texture.h"

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp> 
#include <opencv2/imgcodecs/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

namespace Immortal
{

Frame::Frame(const std::string & path, int channels, Format format)
{
    if (!strcmp(path.c_str() + path.size() - 4, ".bmp"))
    {
        ReadByInternal(path);
        return;
    }
    ReadByOpenCV(path);
}

Frame::Frame(const std::string &path, bool flip)
{
    ReadByOpenCV(path, flip);
}

Frame::Frame(uint32_t width, uint32_t height, int depth, const void *data) :
    width{ width },
    height{ height },
    depth{ depth }
{

}

bool Frame::Read(const std::string &path, cv::Mat &outputMat)
{
    cv::Mat dst;
    cv::Mat	src = cv::imread(path, cv::IMREAD_UNCHANGED);
    if (!src.data)
    {
        LOG::ERR("Failed to load image -> {{0}}", path);
        return false;
    }
    if (src.data)
    {
        depth = src.channels();
        if (depth == 4)
        {
            cv::cvtColor(src, dst, cv::COLOR_BGRA2RGBA);
        }
        else
        {
            cv::cvtColor(src, dst, cv::COLOR_BGR2RGBA);
        }
    }
    width       = dst.cols;
    height      = dst.rows;
    depth       = dst.channels();
    desc.Format = Format::RGBA8;

    if (dst.depth() == CV_16U)
    {
        desc.Format = Format::RGBA16;
    }
    if (dst.depth() == CV_32FC4)
    {
        desc.Format = Format::RGBA32F;
    }
    outputMat = dst;

    spatial = ncast<uint64_t>(width) * ncast<uint64_t>(height);
    size    = spatial * ncast<uint64_t>(depth) * ((desc.Format == Format::RGBA32F) ? sizeof(float) :
                                                 ((desc.Format == Format::RGBA16) ? sizeof(uint16_t ) : sizeof(uint8_t)));
    return true;
}

void Frame::ReadByOpenCV(const std::string &path)
{
    cv::Mat src;
    if (!Read(path, src))
    {
        return;
    }

    data.reset(new uint8_t[size]);
    memcpy(data.get(), src.data, size);
        
    src.release();
}

void Frame::ReadByOpenCV(const std::string &path, bool flip)
{
    cv::Mat src;
    cv::Mat dst;
    if (!Read(path, src))
    {
        return;
    }
    if (flip)
    {
        cv::flip(src, dst, 0);
        src.release();
    }
    else
    {
        dst = src;
    }

    data.reset(new uint8_t[size]);
    memcpy(data.get(), dst.data, size);

    dst.release();
}

void Frame::ReadByInternal(const std::string &path)
{
    Media::BMPDecoder decoder{};
    decoder.Read(path);

    depth       = 4;
    width       = decoder.Width();
    height      = decoder.Height();
    spatial     = ncast<uint64_t>(width) * ncast<uint64_t>(height);
    size        = spatial * ncast<uint64_t>(depth);
    desc.Format = Format::BGRA8;

    decoder.Swap(data);
}

Frame::~Frame()
{

}

}
