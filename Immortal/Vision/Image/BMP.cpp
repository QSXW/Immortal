#include "BMP.h"

#include <iostream>
#include <vector>

#include "FileSystem/Stream.h"
#include "ICC.h"
#include "Vision/Common/ByteStream.h"

namespace Immortal
{
namespace Vision
{

static void PutData(std::vector<uint8_t> &buf, const void *data, size_t size)
{
	size_t pos = buf.size();
	buf.resize(pos + size);
	memcpy(&buf[pos], data, size);
}

static void Skip(std::vector<uint8_t> &buf, size_t size)
{
	buf.resize(buf.size() + size);
}

typedef enum
{
	BMP_RGB  = 0,
	BMP_RLE8 = 1,
	BMP_RLE4 = 2,
	BMP_BITFIELDS = 3,
} BiCompression;

struct BITMAPFILEHEADER
{
#pragma pack(push, 1)
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffbits;
#pragma pack(pop)
};

struct BITMAPV5HEADER
{
	DWORD        bV5Size;
	LONG         bV5Width;
	LONG         bV5Height;
	WORD         bV5Planes;
	WORD         bV5BitCount;
	DWORD        bV5Compression;
	DWORD        bV5SizeImage;
	LONG         bV5XPelsPerMeter;
	LONG         bV5YPelsPerMeter;
	DWORD        bV5ClrUsed;
	DWORD        bV5ClrImportant;
	DWORD        bV5RedMask;
	DWORD        bV5GreenMask;
	DWORD        bV5BlueMask;
	DWORD        bV5AlphaMask;
	DWORD        bV5CSType;
	CIEXYZTRIPLE bV5Endpoints;
	DWORD        bV5GammaRed;
	DWORD        bV5GammaGreen;
	DWORD        bV5GammaBlue;
	DWORD        bV5Intent;
	DWORD        bV5ProfileData;
	DWORD        bV5ProfileSize;
	DWORD        bV5Reserved;
};

BMPCodec::BMPCodec() :
    Super{},
    ICLASS
{

}

BMPCodec::~BMPCodec()
{

}

static void ImageCopy(uint8_t *dst, int dstStride, const uint8_t *src, int srcStride, int width, int height)
{
	for (int i = 0; i < height; i++)
	{
		memcpy(dst, src, width);
		src += srcStride;
		dst += dstStride;
	}
}

static constexpr uint32_t PackRGBA(uint32_t r, uint32_t g, uint32_t b, uint32_t a = 0xff)
{
	return (a << 24) | (b << 16) | (g << 8) | r;
}

static void PAL82RGBA(uint8_t *dst, int dstStride, const uint8_t *src, int srcStride, int width, int height, const uint32_t *palette)
{
	for (int i = 0; i < height; i++)
	{
		const uint8_t *_src = &src[i * srcStride];
		uint32_t *_dst = (uint32_t *) &dst[i * dstStride];
		for (int j = 0; j < width; j++)
		{
			*_dst++ = palette[*_src++];
		}
	}
}

static void Gray2RGBA(uint8_t *dst, int dstStride, const uint8_t *src, int srcStride, int width, int height)
{
	for (int i = 0; i < height; i++)
	{
		const uint8_t *_src = &src[i * srcStride];
		uint32_t *_dst = (uint32_t *) &dst[i * dstStride];
		for (int j = 0; j < width; j++)
		{
			uint8_t v = *_src++;
			*_dst++ = PackRGBA(v, v, v);
		}
	}
}

static void BGR2RGBA(uint8_t *dst, int dstStride, const uint8_t *src, int srcStride, int width, int height)
{
	for (int i = 0; i < height; i++)
	{
		const uint8_t *_src = &src[i * srcStride];
		uint32_t *_dst = (uint32_t *)&dst[i * dstStride];
		for (int j = 0; j < width; j++)
		{
			*_dst++ = PackRGBA(_src[2], _src[1], _src[0]);
			_src += 3;
		}
	}
}

static void BGRX2RGBA(uint8_t *dst, int dstStride, const uint8_t *src, int srcStride, int width, int height)
{
	for (int i = 0; i < height; i++)
	{
		const uint8_t *_src = &src[i * srcStride];
		uint32_t *_dst = (uint32_t *) &dst[i * dstStride];
		for (int j = 0; j < width; j++)
		{
			*_dst++ = PackRGBA(_src[2], _src[1], _src[0]);
			_src += 4;
		}
	}
}

static void RGBX2RGBA(uint8_t *dst, int dstStride, const uint8_t *src, int srcStride, int width, int height)
{
	for (int i = 0; i < height; i++)
	{
		const uint8_t *_src = &src[i * srcStride];
		uint32_t *_dst = (uint32_t *) &dst[i * dstStride];
		for (int j = 0; j < width; j++)
		{
			*_dst++ = PackRGBA(_src[0], _src[1], _src[2]);
			_src += 4;
		}
	}
}

static void RGB5552RGBA(uint8_t *dst, int dstStride, const uint8_t *src, int srcStride, int width, int height)
{
	for (int i = 0; i < height; i++)
	{
		const uint16_t *_src = (const uint16_t *)&src[i * srcStride];
		uint32_t *_dst = (uint32_t *) &dst[i * dstStride];
		for (int j = 0; j < width; j++)
		{
			uint16_t rgb = *_src++;
			uint8_t r5 = (rgb >> 10) & 0x1F;
			uint8_t g5 = (rgb >> 5 ) & 0x1F;
			uint8_t b5 =  rgb        & 0x1F;

			*_dst++ = PackRGBA(
				(r5 << 3) | (r5 >> 2),
				(g5 << 3) | (g5 >> 2),
				(b5 << 3) | (b5 >> 2)
			);
		}
	}
}

static void RGB5652RGBA(uint8_t *dst, int dstStride, const uint8_t *src, int srcStride, int width, int height)
{
	for (int i = 0; i < height; i++)
	{
		const uint16_t *_src = (const uint16_t *)&src[i * srcStride];
		uint32_t *_dst = (uint32_t *) &dst[i * dstStride];
		for (int j = 0; j < width; j++)
		{
			uint16_t rgb = *_src++;
			uint8_t r5 = (rgb >> 11) & 0x1F;
			uint8_t g6 = (rgb >> 5 ) & 0x3F;
			uint8_t b5 =  rgb        & 0x1F;

			*_dst++ = PackRGBA(
			    (r5 << 3) | (r5 >> 2),
			    (g6 << 2) | (g6 >> 4),
			    (b5 << 3) | (b5 >> 2)
			);
		}
	}
}

static void RGB4442RGBA(uint8_t *dst, int dstStride, const uint8_t *src, int srcStride, int width, int height)
{
	for (int i = 0; i < height; i++)
	{
		const uint16_t *_src = (const uint16_t *)&src[i * srcStride];
		uint32_t *_dst = (uint32_t *) &dst[i * dstStride];
		for (int j = 0; j < width; j++)
		{
			uint16_t rgb = *_src++;
			uint8_t r4 = (rgb >> 8) & 0x0F;
			uint8_t g4 = (rgb >> 4) & 0x0F;
			uint8_t b4 =  rgb       & 0x0F;

        	*_dst++ = PackRGBA(
			    (r4 << 4) | r4,
			    (g4 << 4) | g4,
			    (b4 << 4) | b4
			);
		}
	}
}

BMPFormat BMPCodec::GetFormat(int compression, const uint32_t *rgb, int depth, int alpha, bool hasInfo)
{
	switch (depth)
	{
		case 32:
			if (compression == BMP_BITFIELDS)
			{
				if (rgb[0] == 0xFF000000 && rgb[1] == 0x00FF0000 && rgb[2] == 0x0000FF00)
				{
					return alpha ? BMPFormat::ABGR : BMPFormat::XBGR;
				}
				else if (rgb[0] == 0x00FF0000 && rgb[1] == 0x0000FF00 && rgb[2] == 0x000000FF)
				{
					return alpha ? BMPFormat::BGRA : BMPFormat::BGRX;
				}
				else if (rgb[0] == 0x0000FF00 && rgb[1] == 0x00FF0000 && rgb[2] == 0xFF000000)
				{
					return alpha ? BMPFormat::ARGB : BMPFormat::XRGB;
				}
				else if (rgb[0] == 0x000000FF && rgb[1] == 0x0000FF00 && rgb[2] == 0x00FF0000)
				{
					return alpha ? BMPFormat::RGBA : BMPFormat::RGBX;
				}
				else
				{
					CLOG_ERROR("Unknown bitfields {} {} {}", rgb[0], rgb[1], rgb[2]);
					return BMPFormat::Unknown;
				}
			}
			return BMPFormat::BGRA;

		case 24:
			return BMPFormat::BGR24;

		case 16:
			if (compression == BMP_RGB)
			{
				return BMPFormat::RGB555;
			}
			else if (compression == BMP_BITFIELDS)
			{
				if (rgb[0] == 0xF800 && rgb[1] == 0x07E0 && rgb[2] == 0x001F)
				{
					return BMPFormat::RGB565;
				}
				else if (rgb[0] == 0x7C00 && rgb[1] == 0x03E0 && rgb[2] == 0x001F)
				{
					return BMPFormat::RGB555;
				}
				else if (rgb[0] == 0x0F00 && rgb[1] == 0x00F0 && rgb[2] == 0x000F)
				{
					return BMPFormat::RGB444;
				}
				else
				{
					CLOG_ERROR("Unknown bitfields {} {} {}", rgb[0], rgb[1], rgb[2]);
					return BMPFormat::Unknown;
				}
			}
			return BMPFormat::Unknown;

		case 8:
			return hasInfo ? BMPFormat::PAL8 : BMPFormat::GRAY8;

		case 1:
		case 4:
			if (hasInfo)
			{
				return BMPFormat::PAL8;
			}

			CLOG_ERROR("Unknown palette for {}-colour BMP", 1 << depth);
			return BMPFormat::Unknown;

		default:
			CLOG_ERROR("depth {} not supported", depth);
			return BMPFormat::Unknown;
	}
}

CodecError BMPCodec::Decode(const CodedFrame &codedFrame)
{
	auto data = codedFrame.GetData();
	auto size = codedFrame.GetSize();

	ByteStream bs{data, size};
	if (size < 14)
	{
		CLOG_ERROR("buf size too small ({})", size);
		return CodecError::ExternalFailed;
	}

	if (bs.get_byte() != 'B' ||
	    bs.get_byte() != 'M')
	{
		CLOG_ERROR("bad magic number");
		return CodecError::ExternalFailed;
	}

	uint32_t fsize = bs.get_le<uint32_t>();
	if (size < fsize)
	{
		CLOG_ERROR("not enough data ({} < {}), trying to decode anyway", size, fsize);
		fsize = size;
	}

	bs.get_le<uint16_t>(); /* reserved1 */
	bs.get_le<uint16_t>(); /* reserved2 */

	uint32_t hsize  = bs.get_le<uint32_t>();  /* header size */
	uint32_t ihsize = bs.get_le<uint32_t>();  /* more header size */
	if (ihsize + 14LL > hsize)
	{
		CLOG_ERROR("invalid header size {}\n", hsize);
		return CodecError::ExternalFailed;
	}

	/* sometimes file size is set to some headers size, set a real size in that case */
	if (fsize == 14 || fsize == ihsize + 14)
	{
		fsize = size - 2;
	}

	if (fsize <= hsize)
	{
		CLOG_ERROR("Declared file size is less than header size ({} < {})",
		       fsize, hsize);
		return CodecError::CorruptedBitstream;
	}

	uint32_t width, height;
	switch (ihsize)
	{
		case 40:         // windib
		case 56:         // windib v3
		case 64:         // OS/2 v2
		case 108:        // windib v4
		case 124:        // windib v5
			width  = bs.get_le<uint32_t>();
			height = bs.get_le<uint32_t>();
			break;
		case 12:        // OS/2 v1
			width  = bs.get_le<uint16_t>();
			height = bs.get_le<uint16_t>();
			break;
		default:
			CLOG_ERROR("Information header size {}", ihsize);
			return CodecError::CorruptedBitstream;
	}

	/* planes */
	if (bs.get_le<uint16_t>() != 1)
	{
		CLOG_ERROR("invalid BMP header");
		return CodecError::CorruptedBitstream;
	}

	uint32_t depth = bs.get_le<uint16_t>();

	int compression = 0;
	if (ihsize >= 40)
	{
		compression = bs.get_le<uint32_t>();
	}

	if (compression != BMP_RGB && compression != BMP_BITFIELDS && compression != BMP_RLE4 &&
	    compression != BMP_RLE8)
	{
		CLOG_ERROR("BMP coding {} does not supported\n", compression);
		return CodecError::CorruptedBitstream;
	}

	int alpha = 0;
	uint32_t rgb[3] = {0};
	if (compression == BMP_BITFIELDS)
	{
		bs.buf += 20;
		rgb[0] = bs.get_le<uint32_t>();
		rgb[1] = bs.get_le<uint32_t>();
		rgb[2] = bs.get_le<uint32_t>();
		if (ihsize > 40)
		{
			alpha = bs.get_le<uint32_t>();
		}
	}

	BMPFormat format = GetFormat(compression, rgb, depth, alpha, hsize - ihsize - 14 > 0);
	if (format == BMPFormat::Unknown)
	{
		return CodecError::CorruptedBitstream;
	}

	picture = Picture{width, height > 0 ? height : -(unsigned) height, Format::RGBA8, true};
	const uint8_t *buf = data + hsize;
	int dsize = size - hsize;

	///* Line size in file multiple of 4 */
	int n = ((width * depth + 31) / 8) & ~3;

	int imageSize = n * height;
	if (imageSize > dsize && compression != BMP_RLE4 && compression != BMP_RLE8)
	{
		n = (width * depth + 7) / 8;
		if (imageSize > dsize)
		{
			CLOG_ERROR("not enough data ({} < {})", dsize, imageSize);
			return CodecError::CorruptedBitstream;
		}
		CLOG_ERROR("data size too small, assuming missing line alignment");
	}

	auto dst   = picture.GetData();
	int stride = picture.GetStride();

	// RLE may skip decoding some picture areas, so blank picture before decoding
	if (compression == BMP_RLE4 || compression == BMP_RLE8)
	{
		memset(dst, 0, height * stride);
	}

	if (height > 0)
	{
		dst = dst + (height - 1) * stride;
		stride = -stride;
	}
	height = picture.GetHeight();

	std::vector<uint32_t> palette;
	if (format == BMPFormat::PAL8)
	{
		int colors = 1 << depth;
		if (ihsize >= 36)
		{
			ByteStream bs{ &data[46], size - 46 };
			int t = bs.get_le<uint32_t>();
			if (t < 0 || t > (1 << depth))
			{
				CLOG_ERROR("Incorrect number of colors - {} for bitdepth {}", t, depth);
			}
			else if (t)
			{
				colors = t;
			}
		}
		else
		{
			colors = std::min(256u, (hsize - ihsize - 14) / 3);
		}

		palette.resize(colors);

		buf = data + 14 + ihsize; // palette location
		ByteStream bs{ buf, size - (14 + ihsize) };

		// OS/2 bitmap, 3 bytes per palette entry
		if ((hsize - ihsize - 14) < (colors << 2))
		{
			if ((hsize - ihsize - 14) < colors * 3)
			{
				CLOG_ERROR("palette doesn't fit in packet");
				return CodecError::CorruptedBitstream;
			}
			for (int i = 0; i < colors; i++)
			{
				palette[i] = (0xFFU << 24) | bs.get_le24();
			}
		}
		else
		{
			for (int i = 0; i < colors; i++)
			{
				uint32_t v = 0xFFU << 24 | bs.get_le<uint32_t>();
				palette[i] |= (v & 0xff00ff00) | ((v >> 16) & 0xff) | ((v << 16) & 0x00ff0000);
			}
		}
		buf = data + hsize;
	}

	if (compression == BMP_RLE4 || compression == BMP_RLE8)
	{
		return CodecError::UnsupportFormat;
	}

	switch (depth)
	{
		case 1:
			for (int i = 0; i < height; i++)
			{
				int j;

				uint32_t *_dst = (uint32_t *)&dst[i * stride];
				for (j = 0; j < width >> 3; j++)
				{
					_dst[j * 8 + 0] = palette[ buf[j] >> 7];
					_dst[j * 8 + 1] = palette[(buf[j] >> 6) & 1];
					_dst[j * 8 + 2] = palette[(buf[j] >> 5) & 1];
					_dst[j * 8 + 3] = palette[(buf[j] >> 4) & 1];
					_dst[j * 8 + 4] = palette[(buf[j] >> 3) & 1];
					_dst[j * 8 + 5] = palette[(buf[j] >> 2) & 1];
					_dst[j * 8 + 6] = palette[(buf[j] >> 1) & 1];
					_dst[j * 8 + 7] = palette[ buf[j]       & 1];
				}

				int alignment = width & 7;
				for (j = 0; j < alignment; j++)
				{
					_dst[width - alignment + j] = palette[buf[width >> 3] >> (7 - j) & 1];
				}
				buf += n;
			}
			break;
		case 24:
			BGR2RGBA(dst, stride, buf, n, width, height);
			break;

		case 8:
			if (format == BMPFormat::PAL8)
			{
				PAL82RGBA(dst, stride, buf, n, width, height, palette.data());
			}
			else
			{
				Gray2RGBA(dst, stride, buf, n, width, height);
			}
			break;

		case 32:
			if (format == BMPFormat::RGBA || format == BMPFormat::BGRA)
			{
				ImageCopy(dst, stride, buf, n, n, height);
			}
			else if (format == BMPFormat::BGRX)
			{
				BGRX2RGBA(dst, stride, buf, n, width, height);
			}
			else if (format == BMPFormat::RGBX)
			{
				RGBX2RGBA(dst, stride, buf, n, width, height);
			}
			break;

		case 4:
			for (int i = 0; i < height; i++)
			{
				int j;
				uint32_t *_dst = (uint32_t *) &dst[i * stride];
				for (j = 0; j < n; j++)
				{
					_dst[0] = palette[(buf[j] >> 4) & 0xF];
					_dst[1] = palette[ buf[j]       & 0xF];
					_dst += 2;
				}
				buf += n;
			}
			break;

		case 16:
			if (format == BMPFormat::RGB555)
			{
				RGB5552RGBA(dst, stride, buf, n, width, height);
			}
			else if (format == BMPFormat::RGB565)
			{
				RGB5652RGBA(dst, stride, buf, n, width, height);
			}
			else if (format == BMPFormat::RGB444)
			{
				RGB4442RGBA(dst, stride, buf, n, width, height);
			}
			break;
		default:
			CLOG_ERROR("BMP decoder is broken");
			return CodecError::CorruptedBitstream;
	}

	if (format == BMPFormat::BGRA)
	{
		int i;
		for (i = 0; i < height; i++)
		{
			int j;
			uint8_t *_dst = dst + stride * i + 3;
			for (j = 0; j < width; j++)
			{
				if (_dst[4 * j])
					break;
			}
			if (j < width)
				break;
		}
		if (i == height)
		{
			for (i = 0; i < height; i++)
			{
				uint8_t *_dst = dst + stride * i + 3;
				for (int j = 0; j < width; j++)
				{
					_dst[4 * j] = 0xff;
				}
			}
		}
	}

    return CodecError::Success;
}

CodecError BMPCodec::Encode(const Picture &picture, CodedFrame &codedFrame)
{
	int width  = picture.GetWidth();
    int height = picture.GetHeight();
	int depth  = picture.GetFormat().ComponentCount();
    int stride = picture.GetStride();

	uint16_t bitCount = 32;
	int linesize = (width * bitCount + 7) >> 3;
	uint32_t imageSize = linesize * height;

	BITMAPFILEHEADER fh = {
		.bfType      = 0x4D42,
		.bfSize      = 14 + 124 + imageSize,
		.bfReserved1 = 0,
		.bfReserved2 = 0,
		.bfOffbits   = 14 +124,
	};

	BITMAPV5HEADER h = {
		.bV5Size          = 124,
		.bV5Width         = width,
		.bV5Height        = height,
		.bV5Planes        = 1,
		.bV5BitCount      = bitCount,
	    .bV5Compression   = BMP_BITFIELDS,
		.bV5SizeImage     = imageSize,
		.bV5XPelsPerMeter = 0,
		.bV5YPelsPerMeter = 0,
		.bV5ClrUsed       = 0,
		.bV5ClrImportant  = 0,
		.bV5RedMask       = 0x000000ff,
		.bV5GreenMask     = 0x0000ff00,
		.bV5BlueMask      = 0x00ff0000,
		.bV5AlphaMask     = 0xff000000,
		.bV5CSType        = 0x4D424544U,
		.bV5Endpoints     = 0,
		.bV5GammaRed      = 0,
		.bV5GammaGreen    = 0,
		.bV5GammaBlue     = 0,
		.bV5Intent        = LCS_GM_IMAGES	,
		.bV5ProfileData   = 124 + imageSize,
		.bV5ProfileSize   = 0,
		.bV5Reserved      = 0,
	};

    std::vector<uint8_t> buf;
	std::string desc;

	int profilePadding = 0;
	ICCProfile profile{"Assets/ICC/Sony_AdobeRGB_1998.icc"};
	std::vector<uint8_t> icc;
	if (!profile)
	{
		CLOG_ERROR("Failed to load icc file");
	}
	else
	{
		icc = profile.SaveProfileToMemory();
		if (!icc.empty())
		{
			desc = profile.GetDescription();

			int profileSize = (int) icc.size();
			profilePadding = (profileSize & 3);
			if (profilePadding > 0)
			{
				profilePadding = 4 - profilePadding;
			}

			int totalProfileSize = profileSize + profilePadding;
			fh.bfSize        += totalProfileSize;
			h.bV5ProfileSize += totalProfileSize;
		}
	}

	PutData(buf, &fh.bfType,      sizeof(uint16_t));
	PutData(buf, &fh.bfSize,      sizeof(uint32_t));
	PutData(buf, &fh.bfReserved1, sizeof(uint16_t));
	PutData(buf, &fh.bfReserved2, sizeof(uint16_t));
	PutData(buf, &fh.bfOffbits,   sizeof(uint32_t));

	PutData(buf, &h.bV5Size         , sizeof(h.bV5Size         ));
	PutData(buf, &h.bV5Width        , sizeof(h.bV5Width        ));
	PutData(buf, &h.bV5Height       , sizeof(h.bV5Height       ));
	PutData(buf, &h.bV5Planes       , sizeof(h.bV5Planes       ));
	PutData(buf, &h.bV5BitCount     , sizeof(h.bV5BitCount     ));
	PutData(buf, &h.bV5Compression  , sizeof(h.bV5Compression  ));
	PutData(buf, &h.bV5SizeImage    , sizeof(h.bV5SizeImage    ));
	PutData(buf, &h.bV5XPelsPerMeter, sizeof(h.bV5XPelsPerMeter));
	PutData(buf, &h.bV5YPelsPerMeter, sizeof(h.bV5YPelsPerMeter));
	PutData(buf, &h.bV5ClrUsed      , sizeof(h.bV5ClrUsed      ));
	PutData(buf, &h.bV5ClrImportant , sizeof(h.bV5ClrImportant ));
	PutData(buf, &h.bV5RedMask      , sizeof(h.bV5RedMask      ));
	PutData(buf, &h.bV5GreenMask    , sizeof(h.bV5GreenMask    ));
	PutData(buf, &h.bV5BlueMask     , sizeof(h.bV5BlueMask     ));
	PutData(buf, &h.bV5AlphaMask    , sizeof(h.bV5AlphaMask    ));
	PutData(buf, &h.bV5CSType       , sizeof(h.bV5CSType       ));
	PutData(buf, &h.bV5Endpoints    , sizeof(h.bV5Endpoints    ));
	PutData(buf, &h.bV5GammaRed     , sizeof(h.bV5GammaRed     ));
	PutData(buf, &h.bV5GammaGreen   , sizeof(h.bV5GammaGreen   ));
	PutData(buf, &h.bV5GammaBlue    , sizeof(h.bV5GammaBlue    ));
	PutData(buf, &h.bV5Intent       , sizeof(h.bV5Intent       ));
	PutData(buf, &h.bV5ProfileData  , sizeof(h.bV5ProfileData  ));
	PutData(buf, &h.bV5ProfileSize  , sizeof(h.bV5ProfileSize  ));
	PutData(buf, &h.bV5Reserved     , sizeof(h.bV5Reserved     ));

	size_t pos = buf.size();
	buf.resize(pos + linesize * height);

	auto dst = &buf[pos];
	auto src = picture.GetData();
    src += stride * height - stride;

	if (bitCount == 24)
	{
		for (int y = 0; y < height; y++, src -= stride, dst += linesize)
		{
			auto _src = src;
			auto _dst = dst;
			for (int x = 0; x < width; x++, _dst += 3, _src += 4)
			{
				_dst[2] = _src[0];
				_dst[1] = _src[1];
				_dst[0] = _src[2];
			}
		}
	}
	else
	{
		ImageCopy(dst, linesize, src, -stride, width * depth, height);
	}

	if (!icc.empty())
	{
		PutData(buf, icc.data(), icc.size());
		if (profilePadding > 0 && profilePadding < 4)
		{
			static const uint8_t pad[4] = {0};
			PutData(buf, pad, profilePadding);
		}
	}

    codedFrame = CodedFrame{std::move(buf)};

    return CodecError::Success;
}

}
}
