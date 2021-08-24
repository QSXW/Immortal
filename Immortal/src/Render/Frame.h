#pragma once
#include "ImmortalCore.h"

#include "Texture.h"

#include <opencv2/core/core.hpp> 

namespace Immortal {

	class IMMORTAL_API Frame
	{
	public:
		enum class Format : int
		{
			None = 0,
			RGB,
			RGBA,
			RGBA16F,
			RGBA32F,
			RG32F,

			SRGB,

			DEPTH32F,
			Depth24Stencil8,

			Depth = Depth24Stencil8
		};

	public:
		Frame() = default;
		Frame(const std::string &path, int channels = 0, Texture::Format format = Texture::Format::None);
		Frame(const std::string &path, bool flip);
		virtual ~Frame();

		virtual uint32_t Width() const { return mWidth; }
		virtual uint32_t Height() const { return mHeight; }
		virtual Texture::Description Type() const { return mDescription; }
		virtual uint8_t *Data() const { return mData.get(); };
		virtual uint64_t Hash() const { return 0xff; };

	private:
		void Frame::Read(const std::string &path, cv::Mat &outputMat);

	private:
		Texture::Description mDescription;
		uint32_t mWidth;
		uint32_t mHeight;
		int mChannels;
		Scope<uint8_t> mData;

	public:
		static Ref<Frame> Create(int format, uint32_t width, uint32_t height, const void *data = nullptr);
		static Ref<Frame> Create(const std::string &filepath);

		static inline uint32_t FormatBitsPerPixel(int format)
		{
			switch (format)
			{
				case (int)Frame::Format::RGB:
				case (int)Frame::Format::SRGB:
				{
					return 3;
				}
				case (int)Frame::Format::RGBA:
				{
					return 4;
				}
				case (int)Frame::Format::RGBA16F:
				{
					return 2 * 4;
				}
				case (int)Frame::Format::RGBA32F:
				{
					return 4 * 4;
				}
				default:
					SLASSERT(false && "The Input format was incorrect.");
					return 0;
			}
		}
	};

}

