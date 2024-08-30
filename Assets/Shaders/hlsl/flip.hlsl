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
    int2 size;
    OutputImage.GetDimensions(size.x, size.y);

    int2 pos = DTid.xy;
#if defined(HFLIP) && defined(VFLIP)
    pos = size - int2(pos.xy);
#elif defined(HFLIP)
    pos = int2(size.x - pos.x, pos.y);
#elif defined(VFLIP)
    pos = int2(pos.x, size.y - pos.y);
#endif

#ifdef TRANSPOSE
    pos = int2(size.y - pos.y, pos.x);
#endif

    float4 pixel  = InputImage0[pos];
    OutputImage[DTid.xy] = pixel;
}
