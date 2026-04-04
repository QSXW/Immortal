// Fullscreen pass: reconstruct lighting from G-buffer (albedo + world normal).

#ifdef __spirv__
#define DEFINE_AS_PUSH_CONSTANT [[vk::push_constant]]
#else
#define DEFINE_AS_PUSH_CONSTANT
#endif

struct DeferredLight
{
	float4 Direction;
	float4 Radiance;
};

struct DeferredResolveParams
{
	DeferredLight Lights[4];
};

DEFINE_AS_PUSH_CONSTANT DeferredResolveParams Params;

struct VSInput
{
	uint vertexId : SV_VertexID;
};

struct PSInput
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};

Texture2D AlbedoMap : register(t0);
Texture2D NormalMap : register(t1);
SamplerState LinearClamp : register(s2);

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
	float3 alb = AlbedoMap.Sample(LinearClamp, input.uv).rgb;
	// Empty G-buffer pixel: keep framebuffer (e.g. sky) — discard so background stays.
	if (dot(alb, alb) < 1e-6)
	{
		discard;
	}

	float3 N = NormalMap.Sample(LinearClamp, input.uv).xyz * 2.0 - 1.0;
	N = normalize(N);

	float3 lit = float3(0, 0, 0);
	for (uint i = 0; i < 4; i++)
	{
		float3 rad = Params.Lights[i].Radiance.rgb;
		if (dot(rad, rad) < 1e-8)
		{
			break;
		}
		float3 L = normalize(Params.Lights[i].Direction.xyz);
		float ndl = saturate(dot(N, L));
		lit += alb * ndl * rad;
	}

	float3 ambient = alb * 0.06;
	return float4(ambient + lit, 1.0);
}
