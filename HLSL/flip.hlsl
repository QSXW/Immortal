/**
 * Copyright (C) 2024, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

RWTexture2D<unorm float4> InputImage0 : register(u0);
RWTexture2D<float4> OutputImage : register(u1);

[numthreads(32, 32, 1)]
void Flip(uint3 DTid : SV_DispatchThreadID)
{
    uint inputWidth;
    uint inputHeight;
    uint outputWidth;
    uint outputHeight;
    InputImage0.GetDimensions(inputWidth, inputHeight);
    OutputImage.GetDimensions(outputWidth, outputHeight);
    if (DTid.x >= outputWidth || DTid.y >= outputHeight)
    {
        return;
    }

    int2 pos = int2(DTid.xy);
#if defined(ROTATE_90)
    pos = int2(int(inputWidth) - 1 - int(DTid.y), int(DTid.x));
#elif defined(ROTATE_180)
    pos = int2(int(inputWidth) - 1 - int(DTid.x), int(inputHeight) - 1 - int(DTid.y));
#elif defined(ROTATE_270)
    pos = int2(int(DTid.y), int(inputHeight) - 1 - int(DTid.x));
#endif

#ifdef HFLIP
    pos.x = int(inputWidth) - 1 - pos.x;
#endif
#ifdef VFLIP
    pos.y = int(inputHeight) - 1 - pos.y;
#endif

    if (any(pos < 0) || pos.x >= int(inputWidth) || pos.y >= int(inputHeight))
    {
        return;
    }

    float4 pixel  = InputImage0[pos];
    OutputImage[DTid.xy] = pixel;
}
