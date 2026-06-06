#include "PNG.h"
#include "Config.h"
#include "ICC.h"
#include "Vision/Video/FFCodec.h"

#if HAVE_PNG
#include <png.h>
#include <lcms2.h>

namespace Immortal
{
namespace Vision
{

struct PngReadState
{
	const uint8_t *data;
	size_t         size;
	size_t         read;
};

struct PngWriteState
{
	std::vector<uint8_t> &data;
};

void ErrorFunc(png_structp png, const char *msg)
{
	LOG_ERROR("[libpng] {}", msg);
	longjmp(png_jmpbuf(png), 1);
};

void WarningFunc(png_structp png, const char *msg)
{
	std::string *err_ptr = static_cast<std::string *>(png_get_error_ptr(png));
	LOG_WARNING("[libpng] {}", msg);
};

Format CAST(int colorType, int bitDepth)
{
	switch (colorType)
	{
	case PNG_COLOR_TYPE_PALETTE:
	case PNG_COLOR_TYPE_RGB:
	case PNG_COLOR_TYPE_RGBA:
		return bitDepth > 8 ? Format::RGBA16 : Format::RGBA8;

		default:
			return Format::None;
	}
}

void WriteCallback(png_structp png, png_bytep data, png_size_t length)
{
	auto state = (PngWriteState *)(png_get_io_ptr(png));
	if (!state)
	{
		LOG_ERROR("NULL PngWriteState");
		return;
	}

	auto pos = state->data.size();
	state->data.resize(pos + length);
	std::memcpy(&state->data[pos], data, length);
}

void FlushCallback(png_structp png)
{
	(void) png;
}

PNGCodec::PNGCodec(const ImageEncodeInfo &info) :
    Super{},
	ICLASS,
    encodeInfo{info}
{

}

PNGCodec::~PNGCodec()
{

}

CodecError PNGCodec::Decode(const CodedFrame &codedFrame)
{
    auto data = codedFrame.GetData();
	auto size = codedFrame.GetSize();

	png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, &ErrorFunc, &WarningFunc);
	if (!png)
	{
		return CodecError::CorruptedBitstream;
	}

	png_infop info = png_create_info_struct(png);
	if (!info)
	{
		png_destroy_read_struct(&png, nullptr, nullptr);
		return CodecError::CorruptedBitstream;
	}

	if (setjmp(png_jmpbuf(png)))
	{
		CLOG_ERROR("libpng error found!");
		png_destroy_read_struct(&png, &info, nullptr);
		return CodecError::CorruptedBitstream;
	}

	PngReadState state{
	    .data   = data,
	    .size   = size,
	    .read   = 0,
	};

	png_set_read_fn(png, &state, [](png_structp png, png_bytep outBytes, png_size_t byteCount) {
		auto *state = (PngReadState *)(png_get_io_ptr(png));
		if (state->read + byteCount > state->size)
		{
			png_error(png, "Read beyond end of data");
			return;
		}
		memcpy(outBytes, &state->data[state->read], byteCount);
		state->read += byteCount;
	});

	png_read_info(png, info);

	png_uint_32 width, height;
	int bitDepth, colorType;
	png_get_IHDR(png, info, &width, &height, &bitDepth, &colorType, nullptr, nullptr, nullptr);

	if (bitDepth == 16)
	{
		png_set_swap(png);
	}

	if (colorType == PNG_COLOR_TYPE_PALETTE)
	{
		png_set_palette_to_rgb(png);
	}

	if (colorType == PNG_COLOR_TYPE_GRAY && bitDepth < 8)
	{
		png_set_expand_gray_1_2_4_to_8(png);
	}

	if (png_get_valid(png, info, PNG_INFO_tRNS))
	{
		png_set_tRNS_to_alpha(png);
	}

	if (colorType == PNG_COLOR_TYPE_RGB ||
	    colorType == PNG_COLOR_TYPE_GRAY ||
	    colorType == PNG_COLOR_TYPE_PALETTE)
	{
		png_set_filler(png, 0xFF, PNG_FILLER_AFTER);
	}

	if (colorType == PNG_COLOR_TYPE_GRAY ||
	    colorType == PNG_COLOR_TYPE_GRAY_ALPHA)
	{
		png_set_gray_to_rgb(png);
	}

	png_read_update_info(png, info);

	picture = Picture{uint32_t(width), uint32_t(height), CAST(PNG_COLOR_TYPE_RGBA, bitDepth), true};

	size_t stride = png_get_rowbytes(png, info);
	auto buf = picture.GetData();

	std::vector<uint8_t> filter;
	filter.resize(stride);

	for (int row = 0; row < height; row++)
	{
		png_bytep rowPtr = &buf[row * picture.GetStride()];
		png_read_row(png, rowPtr, (row == 0) ? nullptr : filter.data());
	}
	png_read_end(png, nullptr);

	png_destroy_read_struct(&png, &info, nullptr);

    return CodecError::Success;
}

CodecError PNGCodec::Encode(const Picture &picture, CodedFrame &codedFrame)
{
	png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
	if (!png)
	{
		CLOG_ERROR("Failed to create png write struct");
		return CodecError::ExternalFailed;
	}

	png_infop info = png_create_info_struct(png);
	if (!info)
	{
		png_destroy_write_struct(&png, nullptr);
		CLOG_ERROR("Failed to create png info struct");
		return CodecError::ExternalFailed;
	}

	if (setjmp(png_jmpbuf(png)))
	{
		png_destroy_write_struct(&png, &info);
		CLOG_ERROR("Failed to set jmp");
		return CodecError::ExternalFailed;
	}

	std::vector<uint8_t> data;
	PngWriteState state = {
	    .data = data,
	};

	png_set_write_fn(png, &state, &WriteCallback, &FlushCallback);

	int color_type = PNG_COLOR_TYPE_RGBA;
	int width  = picture.GetWidth();
	int height = picture.GetHeight();

	Format format = picture.GetFormat();
	int bitDepth = format == Format::RGBA16 ? 16 : 8;

	png_set_IHDR(png, info, width, height,
	             bitDepth, color_type, PNG_INTERLACE_NONE,
	             PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	auto icc = picture.GetProperty<ICCProfileProperty>();
	if (icc)
	{
		auto &profile = icc->profile;
		png_set_iCCP(png, info, icc->description.c_str(), 0, profile.data(), profile.size());
	}

	png_write_info(png, info);

	if (bitDepth == 16)
	{
		png_set_swap(png);
	}

	for (uint32_t r = 0; r < height; r++)
	{
		uint8_t *row = &picture.GetData()[r * picture.GetStride()];
		png_write_row(png, row);
		if (this->encodeInfo.listener)
		{
			this->encodeInfo.listener->SetProgress((float)r / height);
		}
	}

	png_write_end(png, nullptr);
	png_destroy_write_struct(&png, &info);

    codedFrame = CodedFrame{std::move(data)};

    return CodecError::Success;
}

}
}
#endif
