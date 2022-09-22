/**
 * Copyright (C) 2021-2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

RWTexture2D<float4> InputImage  : register(u0);
RWTexture2D<float4> OutputImage : register(u1);

SamplerState Sampler : register(s0);

struct Properties
{
    float4 Color;
    float4 HSL;

    float ColorTemperature;
    float Hue;

    float White;
    float Black;

    float Exposure;
    float Contrast;
    float Hightlights;
    float Shadow;
    float Vividness;
};

ConstantBuffer<Properties> push_constant : register(b8);
#define properties push_constant

float avg(float3 color)
{
    return (color.r + color.g + color.b) / 3;
}

float3 rgb2hsv(float3 c)
{
    float4 K = float4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    float4 p = lerp(float4(c.bg, K.wz), float4(c.gb, K.xy), step(c.b, c.g));
    float4 q = lerp(float4(p.xyw, c.r), float4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return float3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

float3 hsv2rgb(float3 c)
{
    float4 K = float4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    float3 p = abs(frac(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * lerp(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

[numthreads(16, 16, 1 )]
void main( uint3 Gid : SV_GroupID, uint GI : SV_GroupIndex, uint3 GTid : SV_GroupThreadID, uint3 DTid : SV_DispatchThreadID )
{
    float4 res = InputImage[DTid.xy];

    res += properties.Color;

    res.xyz = clamp(res.xyz + properties.White * res.xyz * 0.5f,          0.0f, 1.0f);
    res.xyz = clamp(res.xyz + properties.Black * (1.0f - res.xyz) * 0.5f, 0.0f, 1.0f);

    float factor = 1.0;
    if (properties.Exposure > 0.0)
    {
        factor = 1.2;
    }

    float3 hsv = rgb2hsv(res.xyz);
    hsv.x  += properties.HSL.x;
    hsv.yz *= properties.HSL.yz + 1.0f;

    hsv.x = fmod(hsv.x, 1.0f);
    hsv.yz = clamp(hsv.yz, 0.0f, 1.0f);

    res.xyz = hsv2rgb(hsv);

    /* Contrast */
    res.rgb = ((res.rgb - 0.5f) * max(1.0f + properties.Contrast, 0)) + 0.5f;

    res.rgb = pow(res.rgb, float(1 / (1.0 + factor * (properties.Exposure - properties.Contrast))).xxx);

    OutputImage[DTid.xy] = res;
}
