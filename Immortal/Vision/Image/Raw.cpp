#include "Raw.h"

#include <libraw/libraw.h>
#include <libraw/libraw_internal.h>
#include <concepts>

namespace Immortal
{
namespace Vision
{

template <class T>
concept SampleType = std::is_same_v<T, uint8_t> || std::is_same_v<T, uint16_t>;

unsigned short curve[0x10000];
template <SampleType T>
void ReadPixelsToPicture(uint8_t *_dst, int dstStride, T *pixels, uint32_t width, uint32_t height, int channels)
{
    for (int i = 0; i < height; i++)
    {
        T *src = &pixels[i * width * channels];
		T *dst = (T *)&_dst[i * dstStride];
        for (int j = 0; j < width; j++, dst += 4, src += 3)
        {
			dst[0] = src[0];
			dst[1] = src[1];
			dst[2] = src[2];
            dst[3] = T(-1);
        }
    }
}

RawCodec::RawCodec(Format outputFormat) :
    processor{},
    format{ outputFormat }
{

}

RawCodec::~RawCodec()
{
	processor.reset();
}

CodecError RawCodec::Decode(const CodedFrame &codedFrame)
{
    const auto &buffer = codedFrame.GetBuffer();

    processor = std::make_shared<LibRaw>();
	if (processor->open_buffer(buffer.data(), buffer.size()) != LIBRAW_SUCCESS)
	{
		LOG::ERR("Failed to open buffer(0x{}) - size({})", (void *)buffer.data(), buffer.size());
		return CodecError::ExternalFailed;
	}

	if (processor->unpack() != LIBRAW_SUCCESS)
	{
		LOG::ERR("Failed to unpack bayer layer");
		return CodecError::ExternalFailed;
	}

    if (format == Format::BayerLayerRGGB)
    {
		auto &rawdata = processor->imgdata.rawdata;
		auto &rawParams = rawdata.iparams;

		auto &sizes = rawdata.sizes;
		auto width  = rawdata.sizes.iwidth;
		auto height = rawdata.sizes.iheight;
		picture = Picture{ width, height, Format::BayerLayerRGGB };

		picture.SetData(((uint8_t *)(rawdata.raw_image + sizes.left_margin)) + sizes.raw_pitch * sizes.top_margin);
		picture.SetStride(0, sizes.raw_pitch);
	
        std::shared_ptr<LibRaw> ref = processor;
		picture.SetRelease([ref](void *) { ref->recycle(); });
    }
	else if (format == Format::RGBA16 || format == Format::RGBA8)
    {
		processor->imgdata.params.user_qual     = 0;
		processor->imgdata.params.use_auto_wb   = 0;
		processor->imgdata.params.use_camera_wb = 1;
		processor->imgdata.params.no_auto_scale = false;
		processor->imgdata.params.output_bps = format == Format::RGBA16 ? 16 : 8;
		if (processor->dcraw_process() != LIBRAW_SUCCESS)
		{
			LOG::ERR("Failed to run dcraw_process");
			return CodecError::ExternalFailed;
		}

		int err = 0;
		libraw_processed_image_t *image = processor->dcraw_make_mem_image(&err);
        if (err != LIBRAW_SUCCESS)
		{
			LOG::ERR("Failed to make memory image");
		    return CodecError::ExternalFailed;
        }

		if (format == Format::RGBA16)
		{
		    picture = Picture{ image->width, image->height, Format::RGBA16, true };
			ReadPixelsToPicture<uint16_t>(picture.GetData(), picture.GetStride(0), (uint16_t *)image->data, image->width, image->height, image->colors);
		}
		else
		{
			picture = Picture{ image->width, image->height, Format::RGBA8, true };
			ReadPixelsToPicture<uint8_t>(picture.GetData(), picture.GetStride(0), image->data, image->width, image->height, image->colors);
		}

        processor->recycle();
    }
	else
	{
		LOG::ERR("Raw Codec only support BayerLayerRGGB/RGBA16/RGB8 formats");
		return CodecError::UnsupportFormat;
	}

    return CodecError::Success;
}

void RawCodec::GetParams(RawParams *pParams)
{
	auto &color   = processor->imgdata.color;
	auto &colors  = processor->imgdata.idata.colors;
	auto &maximum = color.maximum;
	auto &black   = color.black;

	float dmin = DBL_MAX;
	float dmax = 0;

	auto &mul     = color.pre_mul;
	auto &pre_mul = color.pre_mul;
	auto &cam_mul = color.cam_mul;

	auto &cblack  = color.cblack;
	auto &white   = color.white;

	processor->subtract_black();
	for (int c = 0; c < SL_ARRAY_LENGTH(pParams->black); c++)
	{
		pParams->black[c] = cblack[c] / 65535.0;
	}

	bool use_camera_wb = true;
	if (use_camera_wb && cam_mul[0] > 0.00001f)
	{
		uint32_t sum[8];
		memset(sum, 0, sizeof sum);
		for (int row = 0; row < 8; row++)
		{
			for (int col = 0; col < 8; col++)
			{
				int c = processor->FC(row, col);
				int val = white[row][col] - cblack[c];
				if (val > 0)
				{
					sum[c] += val;
				}
				sum[c + 4]++;
			}
		}
		if (color.as_shot_wb_applied)
		{
			// Nikon sRAW: camera WB already applied:
			pre_mul[0] = pre_mul[1] = pre_mul[2] = pre_mul[3] = 1.0;
		}
		else if (sum[0] && sum[1] && sum[2] && sum[3])
		{
			for (int c = 0; c < 4; c++)
			{
				mul[c] = (float)sum[c + 4] / sum[c];
			}
		}
		else if (cam_mul[0] > 0.00001f && cam_mul[2] > 0.00001f)
		{
			memcpy(pre_mul, cam_mul, sizeof pre_mul);
		}
		else
		{
			processor->imgdata.process_warnings |= LIBRAW_WARN_BAD_CAMERA_WB;
		}
	}

	if (mul[1] == 0)
	{
		mul[1] = 1;
	}
	if (mul[3] == 0)
	{
		mul[3] = colors < 4 ? mul[1] : 1;
	}
	if (mul[1] == 0)
	{
		mul[1] = 1;
	}
	if (mul[3] == 0)
	{
		mul[3] = colors < 4 ? mul[1] : 1;
	}

	maximum -= black;
	for (int c = 0; c < SL_ARRAY_LENGTH(mul); c++)
	{
		dmin = std::min(dmin, mul[c]);
		dmax = std::max(dmax, mul[c]);
	}

	if (true /* !hightlight */)
	{
		dmax = dmin;
	}

	if (dmax > 0.00001 && maximum > 0)
	{
		for (int c = 0; c < SL_ARRAY_LENGTH(mul); c++)
		{
			pParams->scale[c] = (mul[c] /= dmax) * 65535.0 / maximum;
		}
	}
	else
	{
		pParams->scale[0] = 1.0f;
		pParams->scale[1] = 1.0f;
		pParams->scale[2] = 1.0f;
		pParams->scale[3] = 1.0f;
	}
}

void RawCodec::GetProjectionMatrix(float matrix[4][4])
{
	static const double(*out_rgb[])[3] = {
	    LibRaw_constants::rgb_rgb,
		LibRaw_constants::adobe_rgb,
	    LibRaw_constants::wide_rgb,
		LibRaw_constants::prophoto_rgb,
	    LibRaw_constants::xyz_rgb,
		LibRaw_constants::aces_rgb,
	    LibRaw_constants::dcip3d65_rgb,
		LibRaw_constants::rec2020_rgb
	};

	auto &color  = processor->imgdata.color;
	auto &colors = processor->imgdata.idata.colors;
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < colors; j++)
		{
			for (int k = matrix[i][j] = 0; k < 3; k++)
			{
				matrix[i][j] += out_rgb[0][i][k] * color.rgb_cam[k][j];
			}
		}
	}

	if (colors == 3)
	{
		matrix[3][3] = 1.0f;
	}
}

#define SQR(x) ((x) * (x))

void RawCodec::GetCurve(float *curve)
{
	auto &color = processor->imgdata.color;
	auto &gamm = processor->imgdata.params.gamm;

	int i;
	double g[6], bnd[2] = {0, 0}, r;
	int imax = (8192 << 3) / processor->imgdata.params.bright;
	float pwr = 0.45 * 0.5;
	float ts  = 4.5;
	int mode = 2;

	g[0] = pwr;
	g[1] = ts;
	g[2] = g[3] = g[4] = 0;
	bnd[g[1] >= 1] = 1;
	if (g[1] && (g[1] - 1) * (g[0] - 1) <= 0)
	{
		for (i = 0; i < 48; i++)
		{
			g[2] = (bnd[0] + bnd[1]) / 2;
			if (g[0])
			bnd[(pow(g[2] / g[1], -g[0]) - 1) / g[0] - 1 / g[2] > -1] = g[2];
			else
			bnd[g[2] / exp(1 - 1 / g[2]) < g[1]] = g[2];
		}
		g[3] = g[2] / g[1];
		if (g[0])
			g[4] = g[2] * (1 / g[0] - 1);
	}
	if (g[0])
		g[5] = 1 / (g[1] * SQR(g[3]) / 2 - g[4] * (1 - g[3]) +
					(1 - pow(g[3], 1 + g[0])) * (1 + g[4]) / (1 + g[0])) -
				1;
	else
		g[5] = 1 / (g[1] * SQR(g[3]) / 2 + 1 - g[2] - g[3] -
					g[2] * g[3] * (log(g[3]) - 1)) -
				1;
	if (!mode--)
	{
		memcpy(gamm, g, sizeof gamm);
		return;
	}
	for (i = 0; i < 0x10000; i++)
	{
		curve[i] = 0xffff;
		if ((r = (double)i / imax) < 1)
			color.curve[i] =
				0x10000 *
				(mode ? (r < g[3] ? r * g[1]
								: (g[0] ? pow(r, g[0]) * (1 + g[4]) - g[4]
										: log(r) * g[2] + 1))
					: (r < g[2] ? r / g[1]
								: (g[0] ? pow((r + g[4]) / (1 + g[4]), 1 / g[0])
										: exp((r - 1) / g[2]))));
	}

	for (int i = 0; i < SL_ARRAY_LENGTH(color.curve); i++)
	{
		curve[i] = (float)color.curve[i] / 65535.0;
	}
}

int RawCodec::GetFlipType()
{
	return processor->imgdata.sizes.flip;
}

}
}
