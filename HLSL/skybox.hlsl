struct PushConstant
{
	float4x4 inverseViewProjection;
};

[[vk::push_constant]] PushConstant pushConstant;

struct PSInput
{
	float4 Pos : SV_POSITION;
	float3 UVW : TEXCOORD0;
};

PSInput VSMain(uint vertexId : SV_VertexID)
{
	float2 uv = float2((vertexId << 1) & 2, vertexId & 2);
	float2 ndc = uv * float2(2.0, -2.0) + float2(-1.0, 1.0);

	PSInput output;
	output.Pos = float4(ndc, 0.0, 1.0);

	float4 worldDir = mul(pushConstant.inverseViewProjection, float4(ndc, 0.0, 1.0));
	output.UVW = worldDir.xyz / worldDir.w;

	return output;
}

TextureCube Texture  : register(t0);
SamplerState Sampler : register(s1);

struct PSOutput
{
	float4 color : SV_TARGET0;
	uint2 pick : SV_TARGET1;
};

PSOutput PSMain(PSInput input)
{
	PSOutput output;
	float3 dir = normalize(input.UVW);
	float3 color = Texture.Sample(Sampler, dir).rgb;
	output.color = float4(max(color, float3(0, 0, 0)), 1.0f);
	output.pick = uint2(0, 0);
	return output;
}
