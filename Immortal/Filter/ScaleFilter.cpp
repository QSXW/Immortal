/**
 * Copyright (C) 2024, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#include "ScaleFilter.h"

namespace Immortal
{

constexpr double Y_RANGE_OFFSET  = (16.0 / 255.0);
constexpr double UV_RANGE_OFFSET = (128.0 / 255.0);
#define UV_TRANSLATE(u, v) u, v, (((u) + (v)) * -UV_RANGE_OFFSET)

static const Matrix4 kBT709MPEGRange = {
    1.0, UV_TRANSLATE(-1.51500715e-04,  1.57476528e+00) - Y_RANGE_OFFSET,
    1.0, UV_TRANSLATE(-1.87280216e-01, -4.68124625e-01) - Y_RANGE_OFFSET,
    1.0, UV_TRANSLATE( 1.85560969e+00,  1.05739981e-04) - Y_RANGE_OFFSET,
    0.0,       0.0,      0.0,    1.0,
};

static const Matrix4 kBT709FullRange = {
    1.0, UV_TRANSLATE(-1.51500715e-04,  1.57476528e+00),
    1.0, UV_TRANSLATE(-1.87280216e-01, -4.68124625e-01),
    1.0, UV_TRANSLATE( 1.85560969e+00,  1.05739981e-04),
    0.0,       0.0,      0.0,    1.0,
};

static const Matrix4 kBT601MpegRange = {
    1.0,  UV_TRANSLATE(     0.0,    1.402) - Y_RANGE_OFFSET,
    1.0,  UV_TRANSLATE(-0.34414, -0.71414) - Y_RANGE_OFFSET,
    1.0,  UV_TRANSLATE(   1.772,      0.0) - Y_RANGE_OFFSET,
    0.0,       0.0,      0.0,    1.0,
};

static const Matrix4 kBT601FullRange = {
    1.0,  UV_TRANSLATE(     0.0,    1.402),
    1.0,  UV_TRANSLATE(-0.34414, -0.71414),
    1.0,  UV_TRANSLATE(   1.772,      0.0),
    0.0,       0.0,      0.0,    1.0,
};

static const Matrix4 kBT2020MPEGRange = {
    1.0,  UV_TRANSLATE(              0.0,            1.4746) - Y_RANGE_OFFSET,
    1.0,  UV_TRANSLATE(-0.16455312684366, -0.57135312684366) - Y_RANGE_OFFSET,
    1.0,  UV_TRANSLATE(           1.8814,               0.0) - Y_RANGE_OFFSET,
    0.0,       0.0,      0.0,    1.0,
};

static const Matrix4 kBT2020FullRange = {
    1.0,  UV_TRANSLATE(              0.0,            1.4746),
    1.0,  UV_TRANSLATE(-0.16455312684366, -0.57135312684366),
    1.0,  UV_TRANSLATE(           1.8814,               0.0),
    0.0,       0.0,      0.0,    1.0,
};

static const Matrix4 *kTransforms[] = {
    &kBT709MPEGRange,
    &kBT601MpegRange,
    &kBT2020MPEGRange,
    &kBT709FullRange,
    &kBT601FullRange,
    &kBT2020FullRange
};

static void FillComponentFormat(Format format, Format *formats)
{
    if (format == Format::Y210 || format == Format::Y216)
    {
		formats[0] = Format::R16G16B16A16_UNORM;
    }
	else if (format.IsType(Format::NV))
	{
		if (format.IsType(Format::HightBitDepth))
		{
			formats[0] = Format::R16;
			formats[1] = Format::RG16;
		}
		else
		{
			formats[0] = Format::R8;
			formats[1] = Format::RG8;
		}
	}
	else if (format.IsType(Format::YUV))
	{
		if (format.IsType(Format::HightBitDepth))
		{
			format = Format::R16;
		}
		else
		{
			format = Format::R8;
		}
		formats[0] = format;
		formats[1] = format;
		formats[2] = format;
	}
    else
    {
		formats[0] = format;
    }
}

static bool IsRGBA(Format format)
{
	return format == Format::RGBA16 || format == Format::RGBA8;
}

ScaleFilter::ScaleFilter(Device *device, Format srcFormat, Format dstFormat, uint32_t width, uint32_t height, ColorSpace colorSpace, bool fullRange) :
    FilterNode{},
    device{ device },
    srcFormat{ srcFormat },
    dstFormat{ dstFormat },
    colorSpace{colorSpace == ColorSpace::Unspecified ? (colorSpace = ColorSpace::BT709) : colorSpace},
    transformIndex{ 0 }
{
    if (width != 0 && height != 0)
    {
		CreateOutputs(width, height);
    }

    auto &f = dstFormat;
    std::string name = "color_space_yuvp2rgba";
    if (srcFormat.IsType(Format::NV))
    {
		name = "color_space_nv122rgba";
    }
    else if (srcFormat == Format::Y210)
    {
		name = "color_space_y2102rgba";
    }
	else if (IsRGBA(srcFormat) && IsRGBA(f))
	{
		name = "color_space_rgba2rgba";
    }
	else if (f.IsType(Format::YUYV))
    {
		name = "color_space_rgba2yuy2";
    }
    else if (f == Format::P210 || f == Format::P212 || f == Format::P216)
    {
		name = "color_space_rgba2p210";
    }
	else if (f.IsType(Format::NV))
	{
		name = "color_space_rgba2nv12";
	}
    else if (f.IsType(Format::YUV))
    {
        if (f == Format::YUV422P ||
            f == Format::YUV422P10 ||
            f == Format::YUV422P12 ||
            f == Format::YUV422P16)
        {
			name = "color_space_rgba2yuv422";
        }
        else if (f == Format::YUV444P ||
                 f == Format::YUV444P10 ||
                 f == Format::YUV444P12 ||
                 f == Format::YUV444P16)
        {
			name = "color_space_rgba2yuv444";
        }
		else
        {
			name = "color_space_rgba2yuv";
        }
    }

    transformIndex = (int) colorSpace - (int) ColorSpace::BT709 + 3 * !!fullRange;
	memcpy(&transform, kTransforms[transformIndex], sizeof(transform));
    if (dstFormat.IsType(Format::YUV))
    {
		transform = Vector::Inverse(transform);
    }
    if (true)
    {
		Ref<Shader> shader = Graphics::CreateShaderByName(name);
        if (!shader)
        {
			LOG_ERROR("Failed to create shader!");
			return;
        }
		pipeline = device->CreateComputePipeline(shader);
    }
    else
    {
		auto path = Graphics::GetShaderAssetPath() / (name + ".hlsl");
		name = name + "_input_transform";
		pipeline = Graphics::GetPipeline(name);
		if (!pipeline)
        {
			std::string source = Graphics::ReadShaderSource(path);
            if (source.empty())
            {
			    return;
            }

            auto device = Graphics::GetDevice();

            ShaderMacro macro = {
		        .name       = "INPUT_TRANSFORM",
		        .definition = "1",
            };
		    Ref<Shader> shader = device->CreateShader(name, ShaderStage::Compute, source, "main", &macro, 1);
            if (!shader)
            {
			    LOG_ERROR("[ScaleFilter] Error when creating `{}` shader", name);
			    return;
            }
		    pipeline = device->CreateComputePipeline(shader);
        }
    }

    Graphics::StorePipeline(name, pipeline);
    descriptorSet = device->CreateDescriptorSet(pipeline);
	sampler = device->CreateSampler(Filter::Linear, AddressMode::Clamp);
	descriptorSet->Set(4, sampler);
}

ScaleFilter::~ScaleFilter()
{

}

void ScaleFilter::CreateOutputs(uint32_t width, uint32_t height)
{
	Format formats[3] = {};

	SamplingFactor factors[3] = {};
	FillComponentFormat(dstFormat, formats);
	GetSamplingFactor(dstFormat, factors);

	auto &f = dstFormat;
	for (size_t i = 0; i < SL_ARRAY_LENGTH(formats); i++)
	{
		auto &format = formats[i];
		if (format != Format::None)
		{
			uint32_t w = width >> factors[i].x;
			uint32_t h = height >> factors[i].y;
			output.emplace_back(device->CreateTexture(format, w, h, Texture::CalculateMipmapLevels(w, h), 1, TextureType::Storage));
		}
	}
}

void ScaleFilter::Run(const std::vector<Ref<Texture>> &input, AsyncComputeThread *asyncComputeThread)
{
    if (!pipeline)
    {
		return;
    }

    if (output.empty())
	{
		CreateOutputs(input[0]->GetWidth(), input[0]->GetHeight());
	}

    uint32_t slot = 0;
    for (; slot < input.size(); slot++)
    {
        descriptorSet->Set(slot, input[slot]);
    }
	for (size_t i = 0; i < output.size(); i++)
	{
		descriptorSet->Set(slot++, output[i]);
	}

    asyncComputeThread->Execute<RecordingTask>([=, this](uint64_t sync, CommandBuffer *commandBuffer) {
        commandBuffer->SetPipeline(pipeline);
        uint32_t nThreadX = SLALIGN(output[0]->GetWidth()  / 32, 32);
        uint32_t nThreadY = SLALIGN(output[0]->GetHeight() / 32, 32);

        SamplingFactor factors[SamplingFactor::kMaxSublayer] = {};
		GetSamplingFactor(dstFormat, factors);
        if (dstFormat == Format::YUV420P ||
            dstFormat == Format::YUV420P10 ||
            dstFormat == Format::YUV420P12 ||
            dstFormat == Format::YUV420P16)
        {
			nThreadX >>= factors[1].x;
			nThreadY >>= factors[1].y;
        }

        struct PushConstant
        {
			Matrix4 transform;
			float samplingFactor[2];
			float nomalizedFactor;
        };

        float normalizedFactor = 1.0f;
		if (dstFormat.IsType(Format::YUV) &&
            !dstFormat.IsType(Format::YUYV) &&
            !dstFormat.IsType(Format::NV))
		{
			if (dstFormat.IsType(Format::_12Bits))
			{
				normalizedFactor = 4095.0f / 65535.0f;
			}
			else if (dstFormat.IsType(Format::_10Bits))
			{
				normalizedFactor = 1023.0f / 65535.0f;
			}
		}
        else if (srcFormat.IsType(Format::YUV))
        {
            if (srcFormat.IsType(Format::_12Bits))
            {
				normalizedFactor = 16.0f; // 65535.0f / 4095.0f;
            }
            else if (srcFormat.IsType(Format::_10Bits))
            {
				normalizedFactor = 64.0f; //65535.0f / 1023.0f;
            }
        }

        PushConstant pushConstant = {
		    .samplingFactor = {
		        1.0f / (output[0]->GetWidth()  << factors[0].x),
		        1.0f / (output[0]->GetHeight() << factors[0].y),
		    },
		    .nomalizedFactor = normalizedFactor
        };

        void *ps = &pushConstant.samplingFactor;
        uint32_t size = sizeof(pushConstant.samplingFactor) + sizeof(pushConstant.nomalizedFactor);
		if (!(IsRGBA(srcFormat) && IsRGBA(dstFormat)))
		{
			memcpy(&pushConstant.transform, &transform, sizeof(Matrix4));
			size = sizeof(pushConstant);
			ps = &pushConstant;
		}

        commandBuffer->PushConstants(ShaderStage::Compute, ps, size, 0);

        commandBuffer->SetDescriptorSet(descriptorSet);
        commandBuffer->Dispatch(nThreadX, nThreadY, 1);
    });
}

}
