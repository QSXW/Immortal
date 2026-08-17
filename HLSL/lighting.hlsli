#ifndef IMMORTAL_LIGHTING_HLSLI
#define IMMORTAL_LIGHTING_HLSLI

static const float kMeshletAmbient = 0.3;
static const float kLightingPi = 3.14159265358979323846;

#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT       1
#define LIGHT_TYPE_SPOT        2

struct LightItem
{
	float4 Direction;
	float4 Radiance;
	float4 Position;
	float4 SpotParams;
};

// ---------------------------------------------------------------------------
// Shadow helpers
// ---------------------------------------------------------------------------

float2 Shadow_WorldToUV(float4x4 lightVP, float3 worldPos, out float depth)
{
	float4 clip = mul(lightVP, float4(worldPos, 1.0));
	float3 ndc = clip.xyz / clip.w;
	depth = ndc.z;
	return float2(ndc.x, -ndc.y) * 0.5 + 0.5;
}

float Shadow_SamplePCF(
    Texture2D<float> shadowMap,
    SamplerState shadowSamp,
    float4x4 lightVP,
    float3 worldPos)
{
	float depth;
	float2 uv = Shadow_WorldToUV(lightVP, worldPos, depth);

	if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0 || depth < 0.0 || depth > 1.0)
		return 1.0;

	uint w, h;
	shadowMap.GetDimensions(w, h);
	float2 texelSize = 1.0 / float2(w, h);

	float sum = 0.0;
	[unroll] for (int y = -1; y <= 1; ++y)
	{
		[unroll] for (int x = -1; x <= 1; ++x)
		{
			float2 off = float2(x, y) * texelSize;
			float d = shadowMap.Sample(shadowSamp, uv + off).r;
			sum += (d >= depth) ? 1.0 : 0.0;
		}
	}
	return sum / 9.0;
}

float Shadow_SampleCascadeForward(
    Texture2D<float> shadow0,
    Texture2D<float> shadow1,
    Texture2D<float> shadow2,
    Texture2D<float> shadow3,
    SamplerState shadowSamp,
    uint cascadeIdx,
    float4x4 cascadeVP,
    float3 worldPos)
{
	if (cascadeIdx == 0u) return Shadow_SamplePCF(shadow0, shadowSamp, cascadeVP, worldPos);
	if (cascadeIdx == 1u) return Shadow_SamplePCF(shadow1, shadowSamp, cascadeVP, worldPos);
	if (cascadeIdx == 2u) return Shadow_SamplePCF(shadow2, shadowSamp, cascadeVP, worldPos);
	return Shadow_SamplePCF(shadow3, shadowSamp, cascadeVP, worldPos);
}

uint Shadow_SelectCascade(float viewDepth, float4 splits, uint count)
{
	for (uint i = 0; i < count; ++i)
	{
		if (viewDepth < splits[i])
			return i;
	}
	return max(count, 1u) - 1u;
}

float2 Shadow_UvFromLightNdc(float2 ndcXY)
{
	return float2(ndcXY.x, -ndcXY.y) * 0.5 + 0.5;
}

float Shadow_SampleCmp4(
    Texture2D shadow0,
    Texture2D shadow1,
    Texture2D shadow2,
    Texture2D shadow3,
    uint cascadeIdx,
    SamplerComparisonState shadowCmpSampler,
    float2 uv,
    float compareZ)
{
	if (cascadeIdx == 0u)
		return shadow0.SampleCmpLevelZero(shadowCmpSampler, uv, compareZ);
	if (cascadeIdx == 1u)
		return shadow1.SampleCmpLevelZero(shadowCmpSampler, uv, compareZ);
	if (cascadeIdx == 2u)
		return shadow2.SampleCmpLevelZero(shadowCmpSampler, uv, compareZ);
	return shadow3.SampleCmpLevelZero(shadowCmpSampler, uv, compareZ);
}

float4x4 Shadow_SelectCascadeMatrix(
    uint cascadeIndex,
    uint cascadeCount,
    float4x4 shadowViewProj,
    float4x4 cascadeVP0,
    float4x4 cascadeVP1,
    float4x4 cascadeVP2,
    float4x4 cascadeVP3)
{
	if (cascadeCount == 0u)
		return shadowViewProj;
	if (cascadeIndex == 0u) return cascadeVP0;
	if (cascadeIndex == 1u) return cascadeVP1;
	if (cascadeIndex == 2u) return cascadeVP2;
	return cascadeVP3;
}

float Shadow_CascadeBlendWeight(
    float viewDepth,
    uint cascadeIndex,
    float4 cascadeSplits,
    uint cascadeCount)
{
	if (cascadeCount <= 1u || cascadeIndex + 1u >= cascadeCount)
		return 0.0;
	float splitEnd = cascadeSplits[cascadeIndex];
	float band = splitEnd * 0.1;
	return saturate((viewDepth - (splitEnd - band)) / max(band, 1e-4));
}

float Shadow_SampleCascadeCmp(
    Texture2D shadow0,
    Texture2D shadow1,
    Texture2D shadow2,
    Texture2D shadow3,
    SamplerComparisonState shadowCmpSampler,
    uint cascadeIndex,
    uint cascadeCount,
    float4x4 shadowViewProj,
    float4x4 cascadeVP0,
    float4x4 cascadeVP1,
    float4x4 cascadeVP2,
    float4x4 cascadeVP3,
    float shadowStrength,
    uint shadowEnabled,
    float3 worldPos)
{
	if (shadowEnabled == 0u)
		return 1.0;

	float4x4 shadowVP = Shadow_SelectCascadeMatrix(
	    cascadeIndex, cascadeCount, shadowViewProj, cascadeVP0, cascadeVP1, cascadeVP2, cascadeVP3);

	float4 lightClip = mul(shadowVP, float4(worldPos, 1.0));
	if (lightClip.w <= 0.0)
		return 1.0;

	float3 ndc = lightClip.xyz / lightClip.w;
	float2 uv = Shadow_UvFromLightNdc(ndc.xy);

	if (ndc.z < 0.0 || ndc.z > 1.0 || uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0)
		return 1.0;

	float w, h;
	shadow0.GetDimensions(w, h);
	float2 texel = 1.0 / float2(max(w, 1.0), max(h, 1.0));
	float compareZ = ndc.z;

	float acc = 0.0;
	[unroll] for (int y = -1; y <= 1; ++y)
	{
		[unroll] for (int x = -1; x <= 1; ++x)
		{
			float2 sampleUv = uv + float2(x, y) * texel;
			acc += Shadow_SampleCmp4(shadow0, shadow1, shadow2, shadow3, cascadeIndex, shadowCmpSampler, sampleUv, compareZ);
		}
	}
	float lit = acc / 9.0;
	return lerp(1.0, lit, shadowStrength);
}

/** Full directional CSM visibility with cascade blend, using SamplerComparisonState. */
float Shadow_DirectionalCSM(
    Texture2D shadow0,
    Texture2D shadow1,
    Texture2D shadow2,
    Texture2D shadow3,
    SamplerComparisonState shadowCmpSampler,
    float4x4 viewM,
    float4x4 shadowViewProj,
    float4x4 cascadeVP0,
    float4x4 cascadeVP1,
    float4x4 cascadeVP2,
    float4x4 cascadeVP3,
    float4 cascadeSplits,
    uint cascadeCount,
    uint shadowEnabled,
    float shadowStrength,
    float3 worldPos)
{
	if (shadowEnabled == 0u)
		return 1.0;

	float viewDepth = abs(mul(viewM, float4(worldPos, 1.0)).z);
	uint cascadeIndex = Shadow_SelectCascade(viewDepth, cascadeSplits, cascadeCount);
	float lit0 = Shadow_SampleCascadeCmp(
	    shadow0, shadow1, shadow2, shadow3,
	    shadowCmpSampler,
	    cascadeIndex, cascadeCount,
	    shadowViewProj, cascadeVP0, cascadeVP1, cascadeVP2, cascadeVP3,
	    shadowStrength, shadowEnabled, worldPos);

	float blend = Shadow_CascadeBlendWeight(viewDepth, cascadeIndex, cascadeSplits, cascadeCount);
	if (blend > 0.0)
	{
		uint cc = max(min(cascadeCount, 4u), 1u);
		uint nextCascade = min(cascadeIndex + 1u, cc - 1u);
		float lit1 = Shadow_SampleCascadeCmp(
		    shadow0, shadow1, shadow2, shadow3,
		    shadowCmpSampler,
		    nextCascade, cascadeCount,
		    shadowViewProj, cascadeVP0, cascadeVP1, cascadeVP2, cascadeVP3,
		    shadowStrength, shadowEnabled, worldPos);
		return lerp(lit0, lit1, blend);
	}
	return lit0;
}

// ---------------------------------------------------------------------------
// Frostbite PBR BRDF building blocks
// (ref: "Moving Frostbite to Physically Based Rendering", Lagarde & de Rousiers, SIGGRAPH 2014)
// ---------------------------------------------------------------------------

float Lighting_Pow5(float x)
{
	float x2 = x * x;
	return x2 * x2 * x;
}

float3 Lighting_F_Schlick(float3 F0, float VoH)
{
	return F0 + (1.0 - F0) * Lighting_Pow5(1.0 - VoH);
}

float3 Lighting_F_SchlickR(float cosTheta, float3 F0, float roughness)
{
	return F0 + (max((1.0 - roughness).xxx, F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

float Lighting_D_GGX(float NoH, float perceptualRoughness)
{
	float a = max(perceptualRoughness * perceptualRoughness, 1e-4);
	float a2 = a * a;
	float d = (NoH * NoH) * (a2 - 1.0) + 1.0;
	return a2 / max(kLightingPi * d * d, 1e-6);
}

float Lighting_V_SmithGGXCorrelated(float NoV, float NoL, float perceptualRoughness)
{
	float a = max(perceptualRoughness * perceptualRoughness, 1e-4);
	float a2 = a * a;
	float GGXV = NoL * sqrt(max(NoV * NoV * (1.0 - a2) + a2, 0.0));
	float GGXL = NoV * sqrt(max(NoL * NoL * (1.0 - a2) + a2, 0.0));
	return 0.5 / max(GGXV + GGXL, 1e-5);
}

float Lighting_Fd_DisneyBurley(float NoV, float NoL, float LoH, float linearRoughness)
{
	float f90 = 0.5 + 2.0 * linearRoughness * LoH * LoH;
	float lightScatter = 1.0 + (f90 - 1.0) * Lighting_Pow5(1.0 - NoL);
	float viewScatter = 1.0 + (f90 - 1.0) * Lighting_Pow5(1.0 - NoV);
	return (lightScatter * viewScatter) / kLightingPi;
}

// ---------------------------------------------------------------------------
// Unified PBR entry points — call these from forward & deferred
// ---------------------------------------------------------------------------

/** Split-sum IBL indirect (irradiance + prefiltered spec * BRDF LUT). */
float3 Lighting_IndirectIBL(
    TextureCube irradianceEnv,
    TextureCube prefilterEnv,
    Texture2D brdfLut,
    SamplerState iblSampler,
    float3 albedo,
    float3 N,
    float3 V,
    float perceptualRoughness,
    float metallic,
    uint useIBL,
    float maxSpecularLod)
{
	if (useIBL == 0u)
		return albedo * 0.06 * (1.0 - metallic * 0.5);

	float NoV = saturate(dot(N, V));
	float3 F0 = lerp(float3(0.04, 0.04, 0.04), albedo, metallic);
	float3 F_ibl = Lighting_F_SchlickR(NoV, F0, perceptualRoughness);
	float3 kS = F_ibl;
	float3 kD = (1.0 - kS) * (1.0 - metallic);

	float3 irradiance = irradianceEnv.Sample(iblSampler, N).rgb;
	float3 diffuseIBL = irradiance * kD * albedo;

	float3 R = reflect(-V, N);
	float lod = perceptualRoughness * maxSpecularLod;
	float3 prefiltered = prefilterEnv.SampleLevel(iblSampler, R, lod).rgb;
	float2 brdf = brdfLut.Sample(iblSampler, float2(NoV, perceptualRoughness)).rg;
	float3 specularIBL = prefiltered * (F0 * brdf.x + brdf.y);

	return diffuseIBL + specularIBL;
}

/**
 * Frostbite standard BRDF — single analytic light contribution.
 *   D: GGX / Trowbridge-Reitz
 *   F: Schlick
 *   V: Smith GGX correlated (Heitz)
 *   Diffuse: Disney Burley (energy-friendly)
 */
float3 Lighting_DirectPBR(
    float3 albedo,
    float3 N,
    float3 V,
    float3 L,
    float3 radiance,
    float perceptualRoughness,
    float metallic,
    float shadow)
{
	float3 H = normalize(L + V);
	float NoL = saturate(dot(N, L));
	float NoV = saturate(dot(N, V));
	float NoH = saturate(dot(N, H));
	float VoH = saturate(dot(V, H));
	float LoH = saturate(dot(L, H));

	float rough = clamp(perceptualRoughness, 0.04, 1.0);
	float linearRough = rough * rough;

	float3 F0 = lerp(float3(0.04, 0.04, 0.04), albedo, metallic);
	float3 F = Lighting_F_Schlick(F0, VoH);

	float D = Lighting_D_GGX(NoH, rough);
	float Vis = Lighting_V_SmithGGXCorrelated(NoV, NoL, rough);
	float3 specBrdf = (D * Vis) * F;
	float3 specular = specBrdf * radiance * NoL * shadow;

	float3 kD = (1.0 - F) * (1.0 - metallic);
	float fd = Lighting_Fd_DisneyBurley(NoV, NoL, LoH, linearRough);
	float3 diffuse = kD * albedo * fd * radiance * NoL * shadow;

	return max(specular + diffuse, float3(0, 0, 0));
}

/** Accumulate multiple analytic lights (directional / point / spot). */
float3 Lighting_AccumulateLights(
    LightItem lights[4],
    uint lightCount,
    float3 worldPos,
    float3 N,
    float3 V,
    float3 albedo,
    float perceptualRoughness,
    float metallic,
    float3 indirect,
    float lightShadow[4])
{
	float3 color = indirect;
	for (uint li = 0; li < lightCount; li++)
	{
		uint lightType = (uint)lights[li].Position.w;
		float range = lights[li].Radiance.w;
		float3 radiance = lights[li].Radiance.rgb;

		float3 L;
		float attenuation = 1.0;

		if (lightType == LIGHT_TYPE_POINT || lightType == LIGHT_TYPE_SPOT)
		{
			float3 toLight = lights[li].Position.xyz - worldPos;
			float dist = length(toLight);
			L = toLight / max(dist, 1e-6);
			float falloff = saturate(1.0 - (dist * dist) / max(range * range, 1e-6));
			attenuation = falloff * falloff;

			if (lightType == LIGHT_TYPE_SPOT)
			{
				float3 spotDir = normalize(lights[li].Direction.xyz);
				float cosTheta = dot(-L, spotDir);
				float cosInner = lights[li].SpotParams.x;
				float cosOuter = lights[li].SpotParams.y;
				attenuation *= saturate((cosTheta - cosOuter) / max(cosInner - cosOuter, 1e-6));
			}
		}
		else
		{
			L = normalize(lights[li].Direction.xyz);
		}

		float3 rad = radiance * attenuation;
		color += Lighting_DirectPBR(albedo, N, V, L, rad, perceptualRoughness, metallic, lightShadow[li]);
	}
	return color;
}

float3 Lighting_PhongLit(float3 albedo, float3 N, float3 L, float3 radiance, float shadow, float ambient)
{
	float NdotL = max(dot(N, L), 0.0);
	float3 diffuse = radiance * NdotL * shadow;
	float3 ambientTerm = radiance * ambient;
	return max((diffuse + ambientTerm) * albedo, float3(0, 0, 0));
}

float3 Lighting_BlinnPhongLit(float3 albedo, float3 N, float3 L, float3 V, float3 radiance, float shadow, float ambient)
{
	float NdotL = max(dot(N, L), 0.0);
	float3 diffuse = albedo * radiance * NdotL * shadow;

	float3 H = normalize(L + V);
	float NdotH = saturate(dot(N, H));
	float spec = pow(NdotH, 64.0) * 0.5 * NdotL;
	float3 specular = radiance * spec * shadow;

	float3 ambientTerm = albedo * ambient;
	return ambientTerm + diffuse + specular;
}

float3 Lighting_CelDiffuse(float ndl, float3 litRgb, float3 shadowRgb, float bands)
{
	float t = saturate(ndl);
	float q = floor(t * bands) / max(bands - 1.0, 1.0);
	return lerp(shadowRgb, litRgb, q);
}

// ---------------------------------------------------------------------------
// Sobel edge helpers (for NPR pass)
// ---------------------------------------------------------------------------

float Lighting_SobelDepthEdge(Texture2D depthMap, SamplerState samp, float2 uv, float2 ts)
{
	float tl = depthMap.Sample(samp, uv + float2(-ts.x, -ts.y)).r;
	float t  = depthMap.Sample(samp, uv + float2(   0 , -ts.y)).r;
	float tr = depthMap.Sample(samp, uv + float2( ts.x, -ts.y)).r;
	float l  = depthMap.Sample(samp, uv + float2(-ts.x,     0)).r;
	float r  = depthMap.Sample(samp, uv + float2( ts.x,     0)).r;
	float bl = depthMap.Sample(samp, uv + float2(-ts.x,  ts.y)).r;
	float b  = depthMap.Sample(samp, uv + float2(   0 ,  ts.y)).r;
	float br = depthMap.Sample(samp, uv + float2( ts.x,  ts.y)).r;

	float gx = -tl - 2.0 * l - bl + tr + 2.0 * r + br;
	float gy = -tl - 2.0 * t - tr + bl + 2.0 * b + br;
	return sqrt(gx * gx + gy * gy);
}

float Lighting_SobelNormalEdge(Texture2D normalMap, SamplerState samp, float2 uv, float2 ts)
{
	float3 tl = normalMap.Sample(samp, uv + float2(-ts.x, -ts.y)).xyz;
	float3 t  = normalMap.Sample(samp, uv + float2(   0 , -ts.y)).xyz;
	float3 tr = normalMap.Sample(samp, uv + float2( ts.x, -ts.y)).xyz;
	float3 l  = normalMap.Sample(samp, uv + float2(-ts.x,     0)).xyz;
	float3 r  = normalMap.Sample(samp, uv + float2( ts.x,     0)).xyz;
	float3 bl = normalMap.Sample(samp, uv + float2(-ts.x,  ts.y)).xyz;
	float3 b  = normalMap.Sample(samp, uv + float2(   0 ,  ts.y)).xyz;
	float3 br = normalMap.Sample(samp, uv + float2( ts.x,  ts.y)).xyz;

	float3 gx = -tl - 2.0 * l - bl + tr + 2.0 * r + br;
	float3 gy = -tl - 2.0 * t - tr + bl + 2.0 * b + br;
	return length(gx) + length(gy);
}

#endif /* IMMORTAL_LIGHTING_HLSLI */
