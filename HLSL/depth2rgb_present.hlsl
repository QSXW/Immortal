// Fullscreen: show RGBA from depth2rgb_cs (linearized / boosted grayscale in .rgb).

Texture2D SrcRgb : register(t0);
SamplerState PointClamp : register(s1);

struct PSInput
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};

PSInput VSMain(uint vertexId : SV_VertexID)
{
	float2 uv = float2((vertexId << 1) & 2, vertexId & 2);
	PSInput o;
	o.pos = float4(uv * float2(2.0, -2.0) + float2(-1.0, 1.0), 0.0, 1.0);
	o.uv = uv;
	return o;
}

float4 PSMain(PSInput input) : SV_TARGET
{
	return SrcRgb.Sample(PointClamp, input.uv);
}
