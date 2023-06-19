/**
 * Copyright (C) 2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

RWTexture2D<unorm float4> InputImage0 : register(u0);
RWTexture2D<unorm float4> InputImage1 : register(u1);
RWTexture2D<unorm float4> InputImage2 : register(u2);
RWTexture2D<float4> OutputImage : register(u3);

SamplerState Sampler : register(s0);

cbuffer push_constant : register(b0)
{
    int _ColorSpace;
    int _10Bits;
};

[numthreads(32, 32, 1)]
void main(uint3 Gid : SV_GroupID, uint GI : SV_GroupIndex, uint3 GTid : SV_GroupThreadID, uint3 DTid : SV_DispatchThreadID)
{
    float4 pixel = float4(0, 0, 0, 1.0f);

    pixel.x = InputImage0[DTid.xy].x;
    pixel.y = InputImage1[DTid.xy * 0.5f].x;
    pixel.z = InputImage2[DTid.xy * 0.5f].x;

    if (_10Bits == 1)
    {
        pixel.xyz *= 65536 / 1023;
    }
    float4 res = float4(0, 0, 0, 1.0f);

    float4x4 T_MAT4_BT709 = float4x4(
        1.00000000e+00, -1.51500715e-04,  1.57476528e+00, -8.50051986e-01,
        1.00000000e+00, -1.87280216e-01, -4.68124625e-01,  2.64957323e-01,
        1.00000000e+00,  1.85560969e+00,  1.05739981e-04, -9.90602811e-01,
        0.00000000e+00,  0.00000000e+00,  0.00000000e+00,  1.00000000e+00
    );
    res = mul(T_MAT4_BT709, pixel);

    OutputImage[DTid.xy] = res;
}
