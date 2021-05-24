#include "impch.h"
#include "Frame.h"

#include "Texture.h"

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp> 
#include <opencv2/imgcodecs/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

namespace Immortal {

	Ref<Frame> Frame::Create(int format, uint32_t width, uint32_t height, const void * data)
	{
		return Ref<Frame>();
	}

	Ref<Frame> Frame::Create(const std::string & filepath)
	{
		return CreateRef<Frame>(filepath);
	}

	Frame::Frame(const std::string & path, int channels, Texture::Format format)
		: mWidth(0), mHeight(0), mChannels(0)
	{
		cv::Mat src;
		Read(path, src);

		uint64_t pixelSize = (uint64_t)mWidth * (uint64_t)mHeight * (uint64_t)mChannels * (src.depth() == CV_32F ? sizeof(float) : sizeof(uint8_t));
		uint8_t *pixels = new uint8_t[pixelSize];
		memcpy(pixels, src.data, pixelSize);
		mData.reset(pixels);

		src.release();
	}

	Frame::Frame(const std::string & path, bool flip)
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

		uint64_t pixelSize = (uint64_t)mWidth * (uint64_t)mHeight * (uint64_t)mChannels * (dst.depth() == CV_32F ? sizeof(float) : sizeof(uint8_t));
		uint8_t *pixels = new uint8_t[pixelSize];
		memcpy(pixels, dst.data, pixelSize);
		mData.reset(pixels);

		dst.release();
	}

	void Frame::Read(const std::string &path, cv::Mat &outputMat)
	{
		cv::Mat dst;
		cv::Mat	src = cv::imread(path, cv::IMREAD_UNCHANGED);
		IM_CORE_ASSERT(src.data, "Failed to load image!");

		if (src.data)
		{
			{
				mChannels = src.channels();
				if (mChannels == 4)
				{
					cv::cvtColor(src, dst, cv::COLOR_BGRA2RGBA);
				}
				else
				{
					cv::cvtColor(src, dst, cv::COLOR_BGR2RGBA);
				}
			}
		}
		mWidth = dst.cols;
		mHeight = dst.rows;

		mChannels = dst.channels();
		if (mChannels == 4)
		{
			if (dst.depth() == CV_32F)
			{
				mDescription.Format = Texture::Format::RGBA16F;
			}
			else
			{
				mDescription.Format = Texture::Format::RGBA8;
			}
		}
		else
		{
			if (dst.depth() == CV_32F)
			{
				mDescription.Format = Texture::Format::RGB16F;
			}
			else
			{
				mDescription.Format = Texture::Format::RGB8;
			}
		}

		if (dst.depth() == CV_16U)
		{
			src = dst;
			src.convertTo(dst, CV_32FC4, 1.0 / (float)0xffff);
			src.release();
			mDescription.Format = Texture::Format::RGBA16F;
		}
		outputMat = dst;
	}

	Frame::~Frame()
	{

	}
}