// Deferred PBR resolve: G-buffer packs metallic in albedo.a, roughness in normal.a; depth for world position.
// IBL: t6 irradiance, t7 prefiltered radiance (same cubemap as skybox, mip chain), t8 BRDF LUT (split-sum).

struct DeferredLight
{
	float4 Direction;
	float4 Radiance;
};

cbuffer DeferredResolvePBRFrame : register(b4)
{
	float4x4 InvViewProjection;
	float4 CameraWorld;
	DeferredLight Lights[4];
	uint LightCount;
	uint UseIBL;
	float MaxSpecularLOD;
	float _padFrame;
};

struct PSInput
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};

Texture2D AlbedoMap : register(t0);
Texture2D NormalMap : register(t1);
Texture2D DepthMap : register(t2);
TextureCube IrradianceMap : register(t6);
TextureCube PrefilterMap : register(t7);
Texture2D BRDFLUT : register(t8);
SamplerState LinearClamp : register(s3);

static const float PI = 3.14159265358979323846;

PSInput VSMain(uint vertexId : SV_VertexID)
{
	float2 uv = float2((vertexId << 1) & 2, vertexId & 2);
	PSInput o;
	o.pos = float4(uv * float2(2.0, -2.0) + float2(-1.0, 1.0), 0.0, 1.0);
	o.uv = uv;
	return o;
}

float3 F_Schlick(float3 F0, float VoH)
{
	float f = pow(1.0 - VoH, 5.0);
	return F0 + (1.0 - F0) * f;
}

float D_GGX(float NoH, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float d = (NoH * NoH) * (a2 - 1.0) + 1.0;
	return a2 / max(PI * d * d, 1e-6);
}

float G_SchlickGGX(float NoX, float roughness)
{
	float r = roughness + 1.0;
	float k = (r * r) / 8.0;
	return NoX / max(NoX * (1.0 - k) + k, 1e-6);
}

float G_Smith(float NoV, float NoL, float roughness)
{
	return G_SchlickGGX(NoV, roughness) * G_SchlickGGX(NoL, roughness);
}

float4 PSMain(PSInput input) : SV_TARGET
{
	float4 albSample = AlbedoMap.Sample(LinearClamp, input.uv);
	float3 albedo = albSample.rgb;
	if (dot(albedo, albedo) < 1e-6)
	{
		discard;
	}

	float metallic = albSample.a;
	float4 nrmSample = NormalMap.Sample(LinearClamp, input.uv);
	float3 N = nrmSample.xyz * 2.0 - 1.0;
	N = normalize(N);
	float perceptualRoughness = saturate(nrmSample.a);
	float alpha = max(perceptualRoughness * perceptualRoughness, 0.001);

	float depth = DepthMap.Sample(LinearClamp, input.uv).r;
	float x = input.uv.x * 2.0 - 1.0;
	float y = 1.0 - input.uv.y * 2.0;
	float4 clip = float4(x, y, depth, 1.0);
	float4 worldH = mul(InvViewProjection, clip);
	float3 worldPos = worldH.xyz / max(worldH.w, 1e-6);

	float3 V = normalize(CameraWorld.xyz - worldPos);
	float NoV = saturate(dot(N, V));

	float3 F0 = lerp(float3(0.04, 0.04, 0.04), albedo, metallic);

	float3 Lo = float3(0, 0, 0);
	for (uint li = 0; li < LightCount; li++)
	{
		float3 L = normalize(Lights[li].Direction.xyz);
		float3 H = normalize(V + L);

		float NoL = saturate(dot(N, L));
		float NoH = saturate(dot(N, H));
		float VoH = saturate(dot(V, H));

		float3 F = F_Schlick(F0, VoH);
		float D = D_GGX(NoH, alpha);
		float G = G_Smith(NoV, NoL, perceptualRoughness);
		float3 specular = (D * G * F) / max(4.0 * NoV * NoL, 1e-5);

		float3 kS = F;
		float3 kD = (1.0 - kS) * (1.0 - metallic);
		float3 diffuse = kD * albedo / PI;

		float3 radiance = Lights[li].Radiance.rgb;
		Lo += (diffuse + specular) * radiance * NoL;
	}

	float3 ambient;
	if (UseIBL != 0u)
	{
		float3 Fenv = F_Schlick(F0, NoV);
		float3 kS = Fenv;
		float3 kD = (1.0 - kS) * (1.0 - metallic);

		float3 irradiance = IrradianceMap.Sample(LinearClamp, N).rgb;
		float3 diffuseIBL = irradiance * kD * albedo;

		float3 R = reflect(-V, N);
		float lod = perceptualRoughness * MaxSpecularLOD;
		float3 prefiltered = PrefilterMap.SampleLevel(LinearClamp, R, lod).rgb;
		float2 brdf = BRDFLUT.Sample(LinearClamp, float2(NoV, perceptualRoughness)).rg;
		float3 specularIBL = prefiltered * (F0 * brdf.x + brdf.y);

		ambient = diffuseIBL + specularIBL;
	}
	else
	{
		ambient = albedo * 0.06 * (1.0 - metallic * 0.5);
	}

	return float4(ambient + Lo, 1.0);
}
