// Depth buffer -> RGBA (UAV) for visualization. Push raw [0,1] depth window (min/max) to stretch contrast.

Texture2D<float> SrcDepth : register(t0);
RWTexture2D<float4> OutRgb : register(u1);
SamplerState PointClamp : register(s2);

struct Depth2RgbPC
{
	float rawDepthMin;
	float rawDepthMax;
};

[[vk::push_constant]]
Depth2RgbPC pc;

[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	uint w, h;
	OutRgb.GetDimensions(w, h);
	if (DTid.x >= w || DTid.y >= h)
	{
		return;
	}
	float2 uv = (float2(DTid.xy) + 0.5f) / float2(max(w, 1u), max(h, 1u));
	float d = SrcDepth.SampleLevel(PointClamp, uv, 0);

	float lo = min(pc.rawDepthMin, pc.rawDepthMax);
	float hi = max(pc.rawDepthMin, pc.rawDepthMax);
	float span = max(hi - lo, 1e-5);
	float t = saturate((d - lo) / span);
	float v = 1.0f - t;

	float delta = saturate(1.0f - d);
	float vLog = saturate(-log10(max(delta, 1e-7)) / 6.0f) * step(1e-4f, delta);
	v = max(v, vLog * 0.55f);

	OutRgb[DTid.xy] = float4(v.xxx, 1.0f);
}
