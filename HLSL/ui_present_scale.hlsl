Texture2D<float4>   Src : register(t0);
RWTexture2D<float4> Dst : register(u1);
SamplerState        S : register(s2);

struct PushConstant
{
	uint2 dstSize;
	uint2 pad;
};

PushConstant push_constant;

[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	if (DTid.x >= push_constant.dstSize.x || DTid.y >= push_constant.dstSize.y)
	{
		return;
	}
	float2 uv = (float2(DTid.xy) + 0.5f) / float2(push_constant.dstSize);
	Dst[DTid.xy] = Src.SampleLevel(S, uv, 0);
}
