// Fullscreen: HDR scene (linear) + cheap bloom, then Uncharted2 tonemap + gamma. Pick buffer passthrough.

struct PSInput
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};

cbuffer PostProcessFrame : register(b0)
{
	float BloomThreshold;
	float BloomIntensity;
	float BloomRadiusPx;
	float Exposure;
	float Gamma;
	float3 _padPost;
};

SamplerState LinearClamp : register(s1);
Texture2D HDRScene : register(t2);
Texture2D<uint4> HDRPick : register(t3);

PSInput VSMain(uint vertexId : SV_VertexID)
{
	float2 uv = float2((vertexId << 1) & 2, vertexId & 2);
	PSInput o;
	o.pos = float4(uv * float2(2.0, -2.0) + float2(-1.0, 1.0), 0.0, 1.0);
	o.uv = uv;
	return o;
}

float3 Uncharted2Tonemap(float3 color)
{
	float A = 0.15;
	float B = 0.50;
	float C = 0.10;
	float D = 0.20;
	float E = 0.02;
	float F = 0.30;
	return ((color * (A * color + C * B) + D * E) / (color * (A * color + B) + D * F)) - E / F;
}

struct PSOut
{
	float4 color : SV_TARGET0;
	uint2 pick : SV_TARGET1;
};

PSOut PSMain(PSInput input)
{
	float w, h;
	HDRScene.GetDimensions(w, h);
	float2 px = float2(1.0 / max(w, 1.0), 1.0 / max(h, 1.0));
	float2 s = BloomRadiusPx * px;

	float3 hdr = HDRScene.Sample(LinearClamp, input.uv).rgb;

	/* Soft bloom: blur(max(scene - threshold, 0)). The old 4-neighbor max(neighbor - T) pass
	   acted like a high-pass on silhouettes and produced rim halos instead of outward glow. */
	static const int2 kBloomOffsets9[9] =
	{
		int2(-1, -1), int2(0, -1), int2(1, -1),
		int2(-1, 0), int2(0, 0), int2(1, 0),
		int2(-1, 1), int2(0, 1), int2(1, 1),
	};
	static const float kBloomWts9[9] =
	{
		1.0, 2.0, 1.0,
		2.0, 4.0, 2.0,
		1.0, 2.0, 1.0,
	};
	float3 bloom = 0.0;
	float wsum = 0.0;
	[unroll]
	for (int i = 0; i < 9; i++)
	{
		float2 suv = input.uv + float2(kBloomOffsets9[i]) * s;
		float3 sc = HDRScene.Sample(LinearClamp, suv).rgb;
		float wi = kBloomWts9[i];
		bloom += max(sc - BloomThreshold, 0.0) * wi;
		wsum += wi;
	}
	bloom /= max(wsum, 1e-5);

	float3 c = hdr + bloom * BloomIntensity;

	c = Uncharted2Tonemap(c * Exposure);
	c = c * (1.0 / Uncharted2Tonemap((11.2).xxx));
	float g = Gamma > 1e-4 ? Gamma : 2.2;
	c = pow(max(c, float3(0, 0, 0)), (1.0 / g).xxx);

	int2 tc = clamp(int2(input.uv * float2(w, h)), int2(0, 0), int2(max(int(w) - 1, 0), max(int(h) - 1, 0)));
	uint4 pk4 = HDRPick.Load(int3(tc, 0));
	uint2 pk = pk4.xy;

	PSOut o;
	o.color = float4(c, 1.0);
	o.pick = pk;
	return o;
}
