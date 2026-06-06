/**
 * Copyright (C) 2024, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

RWTexture2D<unorm float4> YUYV : register(u0);
RWTexture2D<float4>       RGBA : register(u1);

#define Y2 yuyv.xz
#define UV yuyv.yw

[numthreads(32, 32, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    float4 yuyv = YUYV[DTid.xy];

    float2 uv0 = DTid.xy;
    uv0.x *= 2;

    float2 uv1 = uv0;
    uv1.x + 1;

    float c0 = dot(UV, float2(-1.51500715e-04,  1.57476528e+00)) + -7.87306888e-01;
    float c1 = dot(UV, float2(-1.87280216e-01, -4.68124625e-01)) +  3.27702421e-01;
    float c2 = dot(UV, float2( 1.85560969e+00,  1.05739981e-04)) + -9.27857713e-01;
    float2 R2 = Y2 + c0.xx;
    float2 G2 = Y2 + c1.xx;
    float2 B2 = Y2 + c2.xx;

    RGBA[uv0] = float4(R2.x, G2.x, B2.x, 1.0f);
    RGBA[uv1] = float4(R2.y, G2.y, B2.y, 1.0f);
}
