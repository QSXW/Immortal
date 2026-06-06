/**
 * Copyright (C) 2024, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

RWTexture2D<unorm float4> InputImage0 : register(u0);
RWTexture2D<float4> OutputImage : register(u1);
RWBuffer<float3> Lut : register(u2);

SamplerState Sampler : register(s0);

struct PushConstant
{
    int lutSize;
	int lutSize2;
	int lutSize3;
};

[[vk::push_constant]] PushConstant pushConstant;

[numthreads(32, 32, 1)]
void InterpolateNearest(uint3 DTid : SV_DispatchThreadID)
{
    float4 pixel  = InputImage0[DTid.xy];
    int lutSize  = pushConstant.lutSize;
    int lutSize2 = pushConstant.lutSize2;
    int lutSize3 = pushConstant.lutSize3;

    int3 rgb = pixel.rgb * lutSize + 0.5;
    int index = rgb.r * lutSize2 + rgb.g * lutSize + rgb.b;

    pixel.rgb = Lut[index].rgb;
    OutputImage[DTid.xy] = pixel;
}


[numthreads(32, 32, 1)]
void InterpolateTrilinear(uint3 DTid : SV_DispatchThreadID)
{
    float4 pixel  = InputImage0[DTid.xy];
    int lutSize  = pushConstant.lutSize;
    int lutSize2 = pushConstant.lutSize2;
    int lutSize3 = pushConstant.lutSize3;

    float3 rgb = pixel.rgb * lutSize;
    int3 prev = rgb;
    int3 next = min(prev + 1, lutSize - 1);

    float3 d = { rgb.r - prev[0], rgb.g - prev[1], rgb.b - prev[2]};
    float3 c000 = Lut[prev[0] * lutSize2 + prev[1] * lutSize + prev[2]];
    float3 c001 = Lut[prev[0] * lutSize2 + prev[1] * lutSize + next[2]];
    float3 c010 = Lut[prev[0] * lutSize2 + next[1] * lutSize + prev[2]];
    float3 c011 = Lut[prev[0] * lutSize2 + next[1] * lutSize + next[2]];
    float3 c100 = Lut[next[0] * lutSize2 + prev[1] * lutSize + prev[2]];
    float3 c101 = Lut[next[0] * lutSize2 + prev[1] * lutSize + next[2]];
    float3 c110 = Lut[next[0] * lutSize2 + next[1] * lutSize + prev[2]];
    float3 c111 = Lut[next[0] * lutSize2 + next[1] * lutSize + next[2]];
    float3 c00  = lerp(c000, c100, d.r);
    float3 c10  = lerp(c010, c110, d.r);
    float3 c01  = lerp(c001, c101, d.r);
    float3 c11  = lerp(c011, c111, d.r);
    float3 c0   = lerp(c00,  c10,  d.g);
    float3 c1   = lerp(c01,  c11,  d.g);
    float3 c    = lerp(c0,   c1,   d.b);

    pixel.rgb = c;
    OutputImage[DTid.xy] = pixel;
}
