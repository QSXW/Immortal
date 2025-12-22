/**
 * Copyright (C) 2022-2024, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

Texture2D<float>    Y    : register(t0);
Texture2D<float>    U    : register(t1);
Texture2D<float>    V    : register(t2);
RWTexture2D<float4> RGBA : register(u3);
SamplerState        S    : register(s4);

#define INPUT_TRANSFORM
struct PushConstant
{
#ifdef INPUT_TRANSFORM
    float4x4 transform;
#endif
    float2 samplingFactor;
    float  nomalizedFactor;
};

[[vk::push_constant]] PushConstant pushConstant;

[numthreads(32, 32, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    float4 pixel = float4(0, 0, 0, 1.0f);
    float2 uv = (float2(DTid.xy) + 0.5f) * pushConstant.samplingFactor;

    pixel.x = Y[DTid.xy];
    pixel.y = U.SampleLevel(S, uv, 0);
    pixel.z = V.SampleLevel(S, uv, 0);

    pixel.xyz *= pushConstant.nomalizedFactor;

#ifdef INPUT_TRANSFORM
#define T_MAT4_BT709 pushConstant.transform
    RGBA[DTid.xy] = mul(pixel, T_MAT4_BT709);
#else
    const double Y_RANGE_OFFSET = (16.0 / 255.0);
    float4x4 T_MAT4_BT709 = float4x4(
        1.0, -1.51500715e-04,  1.57476528e+00, -0.7873068896425 - Y_RANGE_OFFSET,
        1.0, -1.87280216e-01, -4.68124625e-01,     0.3277024205 - Y_RANGE_OFFSET,
        1.0,  1.85560969e+00,  1.05739981e-04, -0.9278577149905 - Y_RANGE_OFFSET,
        0.0,             0.0,             0.0,             1.0
    );
    RGBA[DTid.xy] = mul(T_MAT4_BT709, pixel);
#endif

}
