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

#define BT709_alpha 1.099296826809442
#define BT709_beta 0.018053968510807

namespace TransferFunction
{

static double Unspecified(double Lc)
{
	return Lc;
}

#define Reserved0 Unspecified
#define Reserved  Unspecified

static double BT709(double Lc)
{
	const double a = BT709_alpha;
	const double b = BT709_beta;

	return (0.0 > Lc) ? 0.0 : (b > Lc) ? 4.500 * Lc :
	                                     a * pow(Lc, 0.45) - (a - 1.0);
}

template <double gamma>
static double Gamma(double Lc)
{
	return (0.0 > Lc) ? 0.0 : pow(Lc, 1.0 / gamma);
}

#define SMPTE170M BT709

static double SMPTE240M(double Lc)
{
	const double a = 1.1115;
	const double b = 0.0228;

	return (0.0 > Lc) ? 0.0 : (b > Lc) ? 4.000 * Lc :
	                                     a * pow(Lc, 0.45) - (a - 1.0);
}

#define LINEAR Unspecified

static double LOG(double Lc)
{
	return (0.01 > Lc) ? 0.0 : 1.0 + std::log10(Lc) / 2.0;
}

static double LOG_SQRT(double Lc)
{
	// sqrt(10) / 1000
	return (0.00316227766 > Lc) ? 0.0 : 1.0 + std::log10(Lc) / 2.5;
}

static double IEC61966_2_4(double Lc)
{
	const double a = BT709_alpha;
	const double b = BT709_beta;

	return (-b >= Lc) ? -a * pow(-Lc, 0.45) + (a - 1.0) : (b > Lc) ? 4.500 * Lc :
	                                                                 a * pow(Lc, 0.45) - (a - 1.0);
}

static double BT1361_ECG(double Lc)
{
	const double a = BT709_alpha;
	const double b = BT709_beta;

	return (-0.0045 >= Lc) ? -(a * pow(-4.0 * Lc, 0.45) + (a - 1.0)) / 4.0 : (b > Lc) ? 4.500 * Lc :
	                                                                                    a * pow(Lc, 0.45) - (a - 1.0);
}

static double IEC61966_2_1(double Lc)
{
	const double a = 1.055;
	const double b = 0.0031308;

	return (0.0 > Lc) ? 0.0 : (b > Lc) ? 12.92 * Lc :
	                                     a * pow(Lc, 1.0 / 2.4) - (a - 1.0);
}

#define PQ_c1 (        3424.0 / 4096.0) /* c3-c2 + 1 */
#define PQ_c2 ( 32.0 * 2413.0 / 4096.0)
#define PQ_c3 ( 32.0 * 2392.0 / 4096.0)
#define PQ_m  (128.0 * 2523.0 / 4096.0)
#define PQ_n  ( 0.25 * 2610.0 / 4096.0)

static double SMPTE2084(double Lc)
{
	const double c1 = PQ_c1;
	const double c2 = PQ_c2;
	const double c3 = PQ_c3;
	const double m = PQ_m;
	const double n = PQ_n;
	const double L = Lc / 10000.0;
	const double Ln = pow(L, n);

	return (0.0 > Lc) ? 0.0 : pow((c1 + c2 * Ln) / (1.0 + c3 * Ln), m);
}

static double SMPTE2084Inv(double E)
{
	const double c1 = PQ_c1;
	const double c2 = PQ_c2;
	const double c3 = PQ_c3;
	const double m = PQ_m;
	const double n = PQ_n;
	const double Em = pow(E, 1.0 / m);

	return (c1 > Em) ? 0.0 : 10000.0 * pow((Em - c1) / (c2 - c3 * Em), 1.0 / n);
}

#define DCI_L 48.00
#define DCI_P 52.37

static double SMPTE428(double Lc)
{
	return (0.0 > Lc) ? 0.0 : pow(DCI_L / DCI_P * Lc, 1.0 / 2.6);
}

#define HLG_a 0.17883277
#define HLG_b 0.28466892
#define HLG_c 0.55991073

static double ARIB_STD_B67(double Lc)
{
    // The function uses the definition from HEVC, which assumes that the peak
    // white is input level = 1. (this is equivalent to scaling E = Lc * 12 and
    // using the definition from the ARIB STD-B67 spec)
    const double a = HLG_a;
    const double b = HLG_b;
    const double c = HLG_c;
    return (0.0 > Lc) ? 0.0 :
        (Lc <= 1.0 / 12.0 ? sqrt(3.0 * Lc) : a * log(12.0 * Lc - b) + c);
}

}

using PFN_TransferFunction = decltype(&TransferFunction::BT709);

void GeneratorTransferFunctionLut(ColorTransferCharacteristic type, float *lut, size_t size)
{
	const PFN_TransferFunction transferFunctions[] = {
	    TransferFunction::Reserved0,
	    TransferFunction::BT709,
        TransferFunction::Unspecified,
        TransferFunction::Reserved,
        TransferFunction::Gamma<2.2>,      
        TransferFunction::Gamma<2.8>,      
        TransferFunction::SMPTE170M,    
        TransferFunction::SMPTE240M,    
        TransferFunction::LINEAR,       
        TransferFunction::LOG,          
        TransferFunction::LOG_SQRT,
        TransferFunction::IEC61966_2_4, 
        TransferFunction::BT1361_ECG,   
        TransferFunction::IEC61966_2_1, 
        TransferFunction::BT709,    
        TransferFunction::BT709,    
        TransferFunction::SMPTE2084Inv,
        TransferFunction::SMPTE428,     
        TransferFunction::ARIB_STD_B67
    };

    double max = size - 1;
    for (size_t i = 0; i < size; i++)
    {
		lut[i] = transferFunctions[(int) type](i / max);
    }
}

ScaleFilter::ScaleFilter(Device *device, Format srcFormat, Format dstFormat, uint32_t width, uint32_t height, ColorSpace colorSpace, bool fullRange) :
    FilterNode{},
    srcFormat{ srcFormat },
    colorSpace{ colorSpace },
    transformIndex{ 0 }
{
    output.emplace_back(device->CreateTexture(dstFormat, width, height, Texture::CalculateMipmapLevels(width, height), 1, TextureType::Storage));
    
    std::string name = "color_space_yuvp2rgba";
    if (srcFormat.IsType(Format::NV))
    {
		name = "color_space_nv122rgba";
    }
    else if (srcFormat == Format::Y210)
    {
		name = "color_space_y2102rgba";
    }

    if (colorSpace == ColorSpace::BT709 && !fullRange)
    {
		pipeline = Graphics::GetPipeline(name);
    }
    else
    {
		transformIndex = (int)colorSpace - (int)ColorSpace::BT709 + 3 * !!fullRange;
		std::string cachedName = name + "_input_transform";
		pipeline = Graphics::GetPipeline(cachedName);
		if (!pipeline)
        {
		    std::string source = Graphics::ReadShaderSource(Graphics::GetShaderAssetPath() / (name + ".hlsl"));
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
			Graphics::StorePipeline(cachedName, pipeline);
        }
    }

 //   size_t size = 1 << 8;
 //   if (srcFormat.IsType(Format::_16Bits))
 //   {
	//	size = 1 << 16;
 //   }
 //   else if (srcFormat.IsType(Format::_12Bits))
 //   {
	//	size = 1 << 12;
 //   }
	//else if (srcFormat.IsType(Format::_10Bits)) [[likely]]
 //   {
	//	size = 1 << 10;
 //   }


 //   stagingLut = device->CreateBuffer(BufferType::TransferSource, size * sizeof(float));
	//lut = device->CreateBuffer(BufferType::Storage | BufferType::TransferDestination, size * sizeof(float), MemoryType::Device, sizeof(float));

 //   float *data;
	//stagingLut->Map((void **)&data, size * sizeof(float), 0);
	//GeneratorTransferFunctionLut(ColorTransferCharacteristic::SMPTE2084, data, size);
	//stagingLut->Unmap();

    descriptorSet = device->CreateDescriptorSet(pipeline);
	sampler = device->CreateSampler(Filter::Linear, AddressMode::Wrap);
	descriptorSet->Set(4, sampler);
}

ScaleFilter::~ScaleFilter()
{

}

void ScaleFilter::Run(const std::vector<Ref<Texture>> &input, AsyncComputeThread *asyncComputeThread)
{
    uint32_t slot = 0;
    for (; slot < input.size(); slot++)
    {
        descriptorSet->Set(slot, input[slot]);
    }
    descriptorSet->Set(slot, output[0]);
	//descriptorSet->Set(5, lut);

    Ref<Buffer> ref = stagingLut;

    asyncComputeThread->Execute<RecordingTask>([=, this](uint64_t sync, CommandBuffer *commandBuffer) {
		if (ref)
		{
			commandBuffer->MemoryCopy(lut, 0, ref, 0, lut->GetSize());
		}

        commandBuffer->SetPipeline(pipeline);
        uint32_t nThreadX = SLALIGN(output[0]->GetWidth()  / 32, 32);
        uint32_t nThreadY = SLALIGN(output[0]->GetHeight() / 32, 32);

        struct PushConstant
        {
			Matrix4 transform;
			float samplingFactor[2];
			float nomalizedFactor;
        };

        static const float sampling[] = {
            1.0f,
            0.5f,
        };
            
        SamplingFactor factors[SamplingFactor::kMaxSublayer] = {};
        GetSamplingFactor(srcFormat, factors);

        float normalizedFactor = 1.0f;
        if (!srcFormat.IsType(Format::NV))
        {
            if (srcFormat.IsType(Format::_12Bits))
            {
				normalizedFactor = 65535.0f / 4095.0f;
            }
            else if (srcFormat.IsType(Format::_10Bits))
            {
				normalizedFactor = 65535.0f / 1023.0f;
            }
        }

        PushConstant pushConstant = {
			.samplingFactor = {
			    1.0f / output[0]->GetWidth(),
			    1.0f / output[0]->GetHeight()},
		    .nomalizedFactor = normalizedFactor
        };

        void *ps = &pushConstant.samplingFactor;
        uint32_t size = sizeof(pushConstant.samplingFactor) + sizeof(pushConstant.nomalizedFactor);
		if (transformIndex)
		{
			memcpy(&pushConstant.transform, kTransforms[transformIndex], sizeof(Matrix4));
			size = sizeof(pushConstant);
			ps = &pushConstant;
		}

        commandBuffer->PushConstants(ShaderStage::Compute, ps, size, 0);

        commandBuffer->SetDescriptorSet(descriptorSet);
        commandBuffer->Dispatch(nThreadX, nThreadY, 1);
    });

    if (ref)
    {
		stagingLut = {};
		asyncComputeThread->Execute<ExecutionCompletedTask>([ref] {});
    }
}

}
