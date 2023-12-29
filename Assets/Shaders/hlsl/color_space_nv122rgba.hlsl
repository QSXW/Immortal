/**
 * Copyright (C) 2022-2024, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

RWTexture2D<unorm float4> Y    : register(u0);
RWTexture2D<unorm float4> UV   : register(u1);
RWTexture2D<float4>       RGBA : register(u2);

[numthreads(32, 32, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    float4 pixel = float4(0, 0, 0, 1.0f);

    pixel.x  = Y[DTid.xy].x;
    pixel.yz = UV[DTid.xy * 0.5f].xy;

    float4x4 T_MAT4_BT709 = float4x4(
        1.00000000e+00, -1.51500715e-04,  1.57476528e+00, -7.87306888e-01,
        1.00000000e+00, -1.87280216e-01, -4.68124625e-01,  3.27702421e-01,
        1.00000000e+00,  1.85560969e+00,  1.05739981e-04, -9.27857713e-01,
        0.00000000e+00,  0.00000000e+00,  0.00000000e+00,  1.00000000e+00
    );

    RGBA[DTid.xy] = mul(T_MAT4_BT709, pixel);
}
