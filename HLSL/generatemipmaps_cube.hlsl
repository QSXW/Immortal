// Downsample using TextureCube sampling on the source mip — seamless filtering across face edges
// (like Vulkan cubemap blit). 2x2 box filter in array space cannot cross edges → seam tint on mips.

TextureCube<float4> Src : register(t0);

RWTexture2DArray<float4> Dst : register(u1);

SamplerState Sampler : register(s0);

struct PushConstant
{
	float2 ratio;
};

PushConstant push_constant;

// Face order +X,-X,+Y,-Y,+Z,-Z — matches D3D cubemap and equirect2cube.hlsl GetCubeDirection.
float3 FaceUVToDirection(uint face, float2 uv)
{
	float2 st = uv * 2.0f - 1.0f;
	if (face == 0u)
		return normalize(float3(1.0f, -st.y, -st.x));
	if (face == 1u)
		return normalize(float3(-1.0f, -st.y, st.x));
	if (face == 2u)
		return normalize(float3(st.x, 1.0f, st.y));
	if (face == 3u)
		return normalize(float3(st.x, -1.0f, -st.y));
	if (face == 4u)
		return normalize(float3(st.x, -st.y, 1.0f));
	return normalize(float3(-st.x, -st.y, -1.0f));
}

[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	float2 uv = push_constant.ratio * (float2(DTid.xy) + 0.5f);
	float3 dir = FaceUVToDirection(DTid.z, uv);
	float4 color = Src.SampleLevel(Sampler, dir, 0.0f);
	Dst[DTid] = color;
}
