// Fullscreen G-buffer lighting resolve (Phong / PBR / NPR).
// Compile with -DIMMORTAL_GBUFFER_SCREEN_VARIANT={0,1,2}.
#ifndef IMMORTAL_GBUFFER_SCREEN_VARIANT
#error Define IMMORTAL_GBUFFER_SCREEN_VARIANT as 0 (Phong), 1 (PBR), or 2 (NPR).
#endif

#include "lighting.hlsli"

// ---------------------------------------------------------------------------
// Shared fullscreen VS
// ---------------------------------------------------------------------------

struct GBufferPSInput
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};

GBufferPSInput VSMain(uint vertexId : SV_VertexID)
{
	float2 uv = float2((vertexId << 1) & 2, vertexId & 2);
	GBufferPSInput o;
	o.pos = float4(uv * float2(2.0, -2.0) + float2(-1.0, 1.0), 0.0, 1.0);
	o.uv = uv;
	return o;
}

float3 GBuffer_ReconstructWorldPos(float4x4 invViewProj, float2 uv, float depth)
{
	float x = uv.x * 2.0 - 1.0;
	float y = 1.0 - uv.y * 2.0;
	float4 worldH = mul(invViewProj, float4(x, y, depth, 1.0));
	return worldH.xyz / max(worldH.w, 1e-6);
}

// ===========================================================================
// Variant 0 — Phong
// ===========================================================================
#if IMMORTAL_GBUFFER_SCREEN_VARIANT == 0

struct PhongLight
{
	float4 Direction;
	float4 Radiance;
};

cbuffer LightingGBufferFrameSimple : register(b7)
{
	float4x4 InvViewProjection;
	float4   CameraWorld;
	float4x4 View;
	PhongLight Lights[4];
	uint     LightCount;
	uint     ShadowEnabled;
	float    ShadowBias;
	float    ShadowStrength;
	uint     ShadowCasterLightIndex;
	float4x4 ShadowViewProjection;
	float4x4 ShadowCascadeViewProjection[4];
	float4   ShadowCascadeSplits;
	uint     ShadowCascadeCount;
	float3   _padShadowCascade;
};

Texture2D    AlbedoMap        : register(t0);
Texture2D    NormalMap         : register(t1);
Texture2D    DepthMap          : register(t2);
Texture2D    ShadowCascade0    : register(t3);
Texture2D    ShadowCascade1    : register(t4);
Texture2D    ShadowCascade2    : register(t5);
Texture2D    ShadowCascade3    : register(t6);
SamplerComparisonState ShadowPointClamp : register(s10);
SamplerState LinearClamp       : register(s11);

float4 PSMainPhong(GBufferPSInput input) : SV_TARGET
{
	float3 alb = AlbedoMap.Sample(LinearClamp, input.uv).rgb;
	if (dot(alb, alb) < 1e-6) discard;

	float3 N = normalize(NormalMap.Sample(LinearClamp, input.uv).xyz * 2.0 - 1.0);
	float depth = DepthMap.Sample(LinearClamp, input.uv).r;
	float3 worldPos = GBuffer_ReconstructWorldPos(InvViewProjection, input.uv, depth);
	float3 V = normalize(CameraWorld.xyz - worldPos);

	float3 rgb = float3(0, 0, 0);
	for (uint i = 0; i < LightCount; i++)
	{
		float3 rad = Lights[i].Radiance.rgb;
		if (dot(rad, rad) < 1e-8) continue;
		float3 L = normalize(Lights[i].Direction.xyz);

		float sh = 1.0;
		if (ShadowEnabled != 0u && i == ShadowCasterLightIndex)
		{
			sh = Shadow_DirectionalCSM(
			    ShadowCascade0, ShadowCascade1, ShadowCascade2, ShadowCascade3,
			    ShadowPointClamp, View, ShadowViewProjection,
			    ShadowCascadeViewProjection[0], ShadowCascadeViewProjection[1],
			    ShadowCascadeViewProjection[2], ShadowCascadeViewProjection[3],
			    ShadowCascadeSplits, ShadowCascadeCount,
			    ShadowEnabled, ShadowStrength, worldPos);
		}
		rgb += Lighting_BlinnPhongLit(alb, N, L, V, rad, sh, kMeshletAmbient);
	}
	return float4(rgb, 1.0);
}

// ===========================================================================
// Variant 1 — PBR (Frostbite)
// ===========================================================================
#elif IMMORTAL_GBUFFER_SCREEN_VARIANT == 1

cbuffer LightingGBufferFramePBR : register(b4)
{
	float4x4 InvViewProjection;
	float4 CameraWorld;
	float4x4 View;
	LightItem Lights[4];
	uint LightCount;
	uint UseIBL;
	float MaxSpecularLOD;
	float Exposure;
	float Gamma;
	float _padA[3];
	float4x4 ShadowViewProjection;
	float4x4 ShadowCascadeViewProjection[4];
	float4 ShadowCascadeSplits;
	uint ShadowCascadeCount;
	float3 _padShadowCascade;
	uint ShadowEnabled;
	float ShadowBias;
	float ShadowStrength;
	uint ShadowCasterLightIndex;
};

Texture2D AlbedoMap    : register(t0);
Texture2D NormalMap    : register(t1);
Texture2D DepthMap     : register(t2);
TextureCube IrradianceMap : register(t6);
TextureCube PrefilterMap  : register(t7);
Texture2D BRDFLUT         : register(t8);
Texture2D ShadowCascade0  : register(t9);
Texture2D ShadowCascade1  : register(t10);
Texture2D ShadowCascade2  : register(t11);
Texture2D ShadowCascade3  : register(t12);
Texture2D EmissiveMap      : register(t13);
SamplerState LinearClamp              : register(s3);
SamplerComparisonState ShadowPointClamp : register(s5);

float4 PSMainPBR(GBufferPSInput input) : SV_TARGET
{
	float4 albSample = AlbedoMap.Sample(LinearClamp, input.uv);
	float3 albedo = albSample.rgb;
	if (dot(albedo, albedo) < 1e-6) discard;

	float metallic = albSample.a;
	float4 nrmSample = NormalMap.Sample(LinearClamp, input.uv);
	float3 N = normalize(nrmSample.xyz * 2.0 - 1.0);
	float perceptualRoughness = saturate(nrmSample.a);

	float depth = DepthMap.Sample(LinearClamp, input.uv).r;
	float3 worldPos = GBuffer_ReconstructWorldPos(InvViewProjection, input.uv, depth);
	float3 V = normalize(CameraWorld.xyz - worldPos);

	float3 indirect = Lighting_IndirectIBL(
	    IrradianceMap, PrefilterMap, BRDFLUT, LinearClamp,
	    albedo, N, V, perceptualRoughness, metallic, UseIBL, MaxSpecularLOD);

	float lightShadow[4] = { 1.0, 1.0, 1.0, 1.0 };
	for (uint si = 0; si < LightCount; si++)
	{
		if ((uint)Lights[si].Position.w == LIGHT_TYPE_DIRECTIONAL && ShadowEnabled != 0u && si == ShadowCasterLightIndex)
		{
			lightShadow[si] = Shadow_DirectionalCSM(
			    ShadowCascade0, ShadowCascade1, ShadowCascade2, ShadowCascade3,
			    ShadowPointClamp, View, ShadowViewProjection,
			    ShadowCascadeViewProjection[0], ShadowCascadeViewProjection[1],
			    ShadowCascadeViewProjection[2], ShadowCascadeViewProjection[3],
			    ShadowCascadeSplits, ShadowCascadeCount,
			    ShadowEnabled, ShadowStrength, worldPos);
		}
	}

	float3 color = Lighting_AccumulateLights(
	    Lights, LightCount, worldPos, N, V,
	    albedo, perceptualRoughness, metallic, indirect, lightShadow);
	color += EmissiveMap.Sample(LinearClamp, input.uv).rgb;
	return float4(color, 1.0);
}

// ===========================================================================
// Variant 2 — NPR (Cel / Toon)
// ===========================================================================
#else

cbuffer LightingGBufferFrameNPR : register(b4)
{
	float4x4 InvViewProjection;
	float4   CameraWorld;
	float4x4 View;
	LightItem Lights[4];

	uint  LightCount;
	float ShadowThreshold;
	float ShadowSoftness;
	float LitIntensity;

	float3 ShadowColor;
	float  RimPower;

	float RimIntensity;
	float EdgeDepthThreshold;
	float EdgeNormalThreshold;
	float EdgeIntensity;
	float Exposure;
	float Gamma;
	float _nprPadShadowMat[2];
	float4x4 ShadowViewProjection;
	float4x4 ShadowCascadeViewProjection[4];
	float4 ShadowCascadeSplits;
	uint ShadowCascadeCount;
	float3 _padShadowCascade;
	uint ShadowEnabled;
	float ShadowBias;
	float ShadowStrength;
	uint ShadowCasterLightIndex;
};

Texture2D    AlbedoMap     : register(t0);
Texture2D    NormalMap     : register(t1);
Texture2D    DepthMap      : register(t2);
SamplerState LinearClamp   : register(s3);
SamplerComparisonState ShadowPointClamp : register(s5);
Texture2D    ShadowCascade0 : register(t9);
Texture2D    ShadowCascade1 : register(t10);
Texture2D    ShadowCascade2 : register(t11);
Texture2D    ShadowCascade3 : register(t12);

float4 PSMainNPR(GBufferPSInput input) : SV_TARGET
{
	float4 albSample = AlbedoMap.Sample(LinearClamp, input.uv);
	float3 albedo = albSample.rgb;
	if (dot(albedo, albedo) < 1e-6) discard;

	float3 N = normalize(NormalMap.Sample(LinearClamp, input.uv).xyz * 2.0 - 1.0);
	float depth = DepthMap.Sample(LinearClamp, input.uv).r;
	float3 worldPos = GBuffer_ReconstructWorldPos(InvViewProjection, input.uv, depth);

	float3 V = normalize(CameraWorld.xyz - worldPos);
	float NoV = saturate(dot(N, V));

	float3 litAcc = float3(0, 0, 0);
	for (uint i = 0; i < LightCount; i++)
	{
		uint lightType = (uint)Lights[i].Position.w;
		float range = Lights[i].Radiance.w;
		float attenuation = 1.0;
		float3 L;

		if (lightType == LIGHT_TYPE_POINT || lightType == LIGHT_TYPE_SPOT)
		{
			float3 toLight = Lights[i].Position.xyz - worldPos;
			float dist = length(toLight);
			L = toLight / max(dist, 1e-6);
			float falloff = saturate(1.0 - (dist * dist) / max(range * range, 1e-6));
			attenuation = falloff * falloff;

			if (lightType == LIGHT_TYPE_SPOT)
			{
				float3 spotDir = normalize(Lights[i].Direction.xyz);
				float cosTheta = dot(-L, spotDir);
				float cosInner = Lights[i].SpotParams.x;
				float cosOuter = Lights[i].SpotParams.y;
				attenuation *= saturate((cosTheta - cosOuter) / max(cosInner - cosOuter, 1e-6));
			}
		}
		else
		{
			L = normalize(Lights[i].Direction.xyz);
		}

		float mapShad = 1.0;
		if (lightType == LIGHT_TYPE_DIRECTIONAL && ShadowEnabled != 0u && i == ShadowCasterLightIndex)
		{
			mapShad = Shadow_DirectionalCSM(
			    ShadowCascade0, ShadowCascade1, ShadowCascade2, ShadowCascade3,
			    ShadowPointClamp, View, ShadowViewProjection,
			    ShadowCascadeViewProjection[0], ShadowCascadeViewProjection[1],
			    ShadowCascadeViewProjection[2], ShadowCascadeViewProjection[3],
			    ShadowCascadeSplits, ShadowCascadeCount,
			    ShadowEnabled, ShadowStrength, worldPos);
		}

		float ndl = dot(N, L);
		float3 lightRad = Lights[i].Radiance.rgb * attenuation;
		float3 litRgb = lightRad * albedo * mapShad;
		float3 shadowRgb = albedo * kMeshletAmbient * mapShad;
		litAcc += Lighting_CelDiffuse(ndl, litRgb, shadowRgb, 4.0);
	}

	float rim = pow(1.0 - NoV, RimPower);
	float3 rimC = rim * RimIntensity;

	float w, h;
	DepthMap.GetDimensions(w, h);
	float2 ts = float2(1.0 / w, 1.0 / h);

	float depthEdge = Lighting_SobelDepthEdge(DepthMap, LinearClamp, input.uv, ts);
	float normalEdge = Lighting_SobelNormalEdge(NormalMap, LinearClamp, input.uv, ts);

	float edge = saturate(
	    step(EdgeDepthThreshold, depthEdge) +
	    step(EdgeNormalThreshold, normalEdge));
	edge *= EdgeIntensity;

	float3 result = litAcc + rimC;
	result = lerp(result, float3(0.02, 0.02, 0.02), edge);
	return float4(result, 1.0);
}

#endif
