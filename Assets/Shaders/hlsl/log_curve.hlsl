/**
 * Copyright (C) 2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

RWTexture2D<unorm float4> InputImage0 : register(u0);
RWTexture2D<float4> OutputImage : register(u1);

SamplerState Sampler : register(s0);

#define threshold (171.2102946929 / 1023.0)

float decode_slog3(float pixel)
{
    float output = pixel * 0.9;
    if (pixel >= threshold)
        output = pow(10.0, ((pixel * 1023.0 - 420.0) / 261.5)) * (0.18 + 0.01) - 0.01;
    else
        output = (pixel * 1023.0 - 95.0) * 0.01125000 / (171.2102946929 - 95.0);

    return output;
}

[numthreads(32, 32, 1)]
void DecodeSlog3(uint3 DTid : SV_DispatchThreadID)
{
    float4 pixel  = InputImage0[DTid.xy];

    pixel.x = decode_slog3(pixel.x);
    pixel.y = decode_slog3(pixel.y);
    pixel.z = decode_slog3(pixel.z);
    OutputImage[DTid.xy] = pixel;
}
