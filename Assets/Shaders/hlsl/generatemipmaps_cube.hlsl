Texture2DArray<float4> Src : register(t0);

RWTexture2DArray<float4> Dst : register(u1);

SamplerState Sampler : register(s0);

struct PushConstant
{
    float2 ratio;
};

PushConstant push_constant;

[numthreads(8, 8, 1)]
void GenerateMipMapsCube(uint3 DTid : SV_DispatchThreadID, uint3 GroupId : SV_GroupID)
{
	float2 uv = push_constant.ratio * (DTid.xy + 0.5);
	// float4 color = Src.SampleLevel(Sampler, float3(uv, DTid.z), 0);

	uint mipLevel = GroupId.z;
    uint arraySlice = GroupId.y;

    // Calculate the coordinates in the current mip level
    uint2 coord = DTid.xy;

	// float4 color = Src.SampleLevel(Sampler, float3(uv, arraySlice), 0);
	float4 color = 0.0f;
    color += Src.SampleLevel(Sampler, float3(coord * 2, arraySlice), mipLevel - 1);
    color += Src.SampleLevel(Sampler, float3(coord * 2 + uint2(1, 0), arraySlice), mipLevel - 1);
    color += Src.SampleLevel(Sampler, float3(coord * 2 + uint2(0, 1), arraySlice), mipLevel - 1);
    color += Src.SampleLevel(Sampler, float3(coord * 2 + uint2(1, 1), arraySlice), mipLevel - 1);
    color *= 0.25f;
	Dst[DTid] = color;
}
