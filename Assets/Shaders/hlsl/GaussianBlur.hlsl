/**
 * Copyright (C) 2024, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

Texture2D<unorm float4> Src : register(t0);
RWTexture2D<float4> Dst : register(u1);
Buffer<float> Weights : register(t2);
SamplerState Sampler : register(s3);

struct PushConstant
{
    int blurDirection;
    int kernalSize;
};
[[vk::push_constant]] PushConstant pushConstant;

#define HOFFSET float2(i * offset, 0.0)
#define VOFFSET float2(0.0, i * offset)

[numthreads(16, 16, 1)]
void Blur(uint3 DTid : SV_DispatchThreadID)
{
    int2 size;
    Dst.GetDimensions(size.x, size.y);

    const int2 pos  = DTid.xy;
    float2 uv = pos;
    uv /= size;

    float4 res = float4(0, 0, 0, 1);
    if (pushConstant.blurDirection == 0)
    {
        /* horizontal */
        float offset = 1.0f / size.x;
        res = Src[uv] * Weights[0];
        for (int i = 1; i < pushConstant.kernalSize; i++)
        {
            const float weight = Weights[i];
            res += Src.SampleLevel(Sampler, uv + HOFFSET, 0) * weight;
            res += Src.SampleLevel(Sampler, uv - HOFFSET, 0) * weight;
        }
    }
    else
    {
        /* vertical */
        float offset = 1.0f / size.y;
        res = Src[uv] * Weights[0];
        for (int i = 1; i < pushConstant.kernalSize; i++)
        {
            const float weight = Weights[i];
            res += Src.SampleLevel(Sampler, uv + VOFFSET, 0) * weight;
            res += Src.SampleLevel(Sampler, uv + VOFFSET, 0) * weight;
        }
    }

    Dst[pos] = res;
}
