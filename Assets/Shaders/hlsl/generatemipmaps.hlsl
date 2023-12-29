Texture2D<float4> Src : register(t0);

RWTexture2D<float4> Dst : register(u1);

SamplerState Sampler : register(s0);

struct PushConstant
{
    float2 ratio;
};

PushConstant push_constant;

[numthreads( 8, 8, 1 )]
void GenerateMipMaps(uint3 DTid : SV_DispatchThreadID)
{
	float2 uv = push_constant.ratio * (DTid.xy + 0.5);
	float4 color = Src.SampleLevel(Sampler, uv, 0);

	Dst[DTid.xy] = color;
}
