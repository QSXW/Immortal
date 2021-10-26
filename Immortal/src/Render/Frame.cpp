#include "impch.h"
#include "Frame.h"

#include "Texture.h"

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp> 
#include <opencv2/imgcodecs/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

namespace Immortal
{

Frame::Frame(const std::string & path, int channels, Texture::Format format)
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

Frame::Frame(UINT32 width, UINT32 height, int depth, const void *data) :
    width{ width },
    height{ height },
    depth{ depth }
{

}

void Frame::Read(const std::string &path, cv::Mat &outputMat)
{
    cv::Mat dst;
    cv::Mat	src = cv::imread(path, cv::IMREAD_UNCHANGED);
    if (!src.data)
    {
        LOG::ERR("Failed to load image -> {{0}}", path);
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
    width  = dst.cols;
    height = dst.rows;

    depth = dst.channels();
    if (depth == 4)
    {
        if (dst.depth() == CV_32F)
        {
            desc.Format = Texture::Format::RGBA16F;
        }
        else
        {
            desc.Format = Texture::Format::RGBA8;
        }
    }
    else
    {
        if (dst.depth() == CV_32F)
        {
            desc.Format = Texture::Format::RGB16F;
        }
        else
        {
            desc.Format = Texture::Format::RGB8;
        }
    }

    if (dst.depth() == CV_16U)
    {
        src = dst;
        src.convertTo(dst, CV_32FC4, 1.0 / (float)0xff);
        src.release();
        desc.Format = Texture::Format::RGBA16F;
    }
    outputMat = dst;

    spatial = (UINT64)width * (UINT64)height;
    size    =  spatial * (UINT64)depth;
}

void Frame::ReadByOpenCV(const std::string &path)
{
    cv::Mat src;
    Read(path, src);

    auto pixelSize = size * (src.depth() == CV_32F ? sizeof(float) : sizeof(uint8_t));
    data.reset(new uint8_t[pixelSize]);
    memcpy(data.get(), src.data, pixelSize);
        
    src.release();
}

void Frame::ReadByOpenCV(const std::string &path, bool flip)
{
    cv::Mat src;
    cv::Mat dst;
    Read(path, src);
    if (flip)
    {
        cv::flip(src, dst, 0);
        src.release();
    }
    else
    {
        dst = src;
    }

    auto pixelSize = size * (src.depth() == CV_32F ? sizeof(float) : sizeof(uint8_t));
    data.reset(new uint8_t[pixelSize]);
    memcpy(data.get(), dst.data, pixelSize);

    dst.release();
}

void Frame::ReadByInternal(const std::string &path)
{
    Media::BMPDecoder decoder{};
    decoder.Read(path);

    depth       = 4;
    width       = decoder.Width();
    height      = decoder.Height();
    spatial     = (UINT64)width * (UINT64)height;
    size        =  spatial * (UINT64)depth;
    desc.Format = Texture::Format::BGRA8;

    decoder.Swap(data);
}

Frame::~Frame()
{

}

}
