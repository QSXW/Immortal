/**
 * Copyright (C) 2022-2024, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

RWTexture2D<unorm float4> Y    : register(u0);
RWTexture2D<unorm float4> U    : register(u1);
RWTexture2D<unorm float4> V    : register(u2);
RWTexture2D<float4>       RGBA : register(u3);

struct PushConstant
{
    float2 samplingFactor;
    float  nomalizedFactor;
};

[[vk::push_constant]] PushConstant pushConstant;

[numthreads(32, 32, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    float4 pixel = float4(0, 0, 0, 1.0f);
    float2 uv = DTid.xy * pushConstant.samplingFactor;

    pixel.x = Y[DTid.xy].x;
    pixel.y = U[uv].x;
    pixel.z = V[uv].x;

    pixel.xyz *= pushConstant.nomalizedFactor;
    float4x4 T_MAT4_BT709 = float4x4(
        1.00000000e+00, -1.51500715e-04,  1.57476528e+00, -8.50051986e-01,
        1.00000000e+00, -1.87280216e-01, -4.68124625e-01,  2.64957323e-01,
        1.00000000e+00,  1.85560969e+00,  1.05739981e-04, -9.90602811e-01,
        0.00000000e+00,  0.00000000e+00,  0.00000000e+00,  1.00000000e+00
    );

    RGBA[DTid.xy] = mul(T_MAT4_BT709, pixel);
}
