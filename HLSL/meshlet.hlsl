#ifdef __spirv__
#define DEFINE_AS_PUSH_CONSTANT [[vk::push_constant]]
#else
#define DEFINE_AS_PUSH_CONSTANT
#endif

#include "lighting.hlsli"

struct CameraProperties
{
	float4x4 MVP;
	float4x4 Model;
	uint     objectId;
	uint     submeshIndex;
	float    roughness;
	float    metallic;
	float4   albedoColor;
	float4   emissive;
};

DEFINE_AS_PUSH_CONSTANT CameraProperties Cam;

struct Vertex
{
	float3 Position;
	float3 Normal;
	float3 Tangent;
	float2 Texcoord;
};

struct Meshlet
{
	uint VertexOffset;
	uint TriangleOffset;
	uint VertexCount;
	uint TriangleCount;
};

StructuredBuffer<Vertex>  Vertices        : register(t0);
StructuredBuffer<Meshlet> Meshlets        : register(t1);
StructuredBuffer<uint>    VertexIndices   : register(t2);
StructuredBuffer<uint>    TriangleIndices : register(t3);

SamplerState Sampler           : register(s4);
Texture2D    DiffuseTexture    : register(t5);

cbuffer Scene : register(b6)
{
	float4   LightDirection;
	float4   LightRadiance;
	float4x4 ShadowDepthVP;
	uint     ShadowEnabled;
	uint     ShadowEnablePcf;
	float    ShadowBias;
	float    ShadowStrength;
	uint     ShadowVisualizeRgb;
	float    PadViz0;
	float    PadViz1;
	float    PadViz2;
	uint     UseIBL;
	float    IBLMaxSpecularLOD;
	uint2    IBLPad;
};

cbuffer ShadowCascade : register(b7)
{
	float4x4 CascadeVP[4];
	float4   CascadeSplits;
	uint     CascadeCount;
	uint3    _CascadePad;
	float4x4 View;
	float4   CameraWorldPos;
};

SamplerState     ShadowMapSampler    : register(s8 );
Texture2D<float> ShadowMapTexture_0 : register(t9 );
Texture2D<float> ShadowMapTexture_1 : register(t10 );
Texture2D<float> ShadowMapTexture_2 : register(t11 );
Texture2D<float> ShadowMapTexture_3 : register(t12 );
SamplerState     IBLClampSampler     : register(s14);
TextureCube      IrradianceEnvMap    : register(t15);
TextureCube      PrefilterEnvMap     : register(t16);
Texture2D        BRDFLUTMap          : register(t17);

struct MeshOutput
{
	float4 Position : SV_POSITION;
	float2 Texcoord : TEXCOORD0;
	float3 Normal   : NORMAL0;
	float3 WorldPos : TEXCOORD1;
	float3 Pos      : TEXCOORD2;
	float3 ViewPos  : TEXCOORD3;
};

[outputtopology("triangle")]
[numthreads(128, 1, 1)]
void MSMain(
                 uint       gtid : SV_GroupThreadID,
                 uint       gid  : SV_GroupID,
    out indices  uint3      triangles[128],
    out vertices MeshOutput vertices[64])
{
	Meshlet m = Meshlets[gid];
	SetMeshOutputCounts(m.VertexCount, m.TriangleCount);

	if (gtid < m.TriangleCount)
	{
		uint packed = TriangleIndices[m.TriangleOffset + gtid];
		uint vIdx0  = (packed >> 0) & 0xFF;
		uint vIdx1  = (packed >> 8) & 0xFF;
		uint vIdx2  = (packed >> 16) & 0xFF;
		triangles[gtid] = uint3(vIdx0, vIdx1, vIdx2);
	}

	if (gtid < m.VertexCount)
	{
		uint vertexIndex = m.VertexOffset + gtid;
		vertexIndex = VertexIndices[vertexIndex];

		float3 localPos = Vertices[vertexIndex].Position;
		vertices[gtid].Position = mul(Cam.MVP, float4(localPos, 1.0f));
		float4 worldH = mul(Cam.Model, float4(localPos, 1.0f));
		vertices[gtid].WorldPos = worldH.xyz;
		vertices[gtid].Pos = worldH.xyz;
		vertices[gtid].ViewPos = mul(View, worldH).xyz;
		vertices[gtid].Texcoord = Vertices[vertexIndex].Texcoord;
		float3 nW = mul((float3x3)Cam.Model, Vertices[vertexIndex].Normal);
		vertices[gtid].Normal = normalize(nW);
	}
}

struct PSOutputForward
{
	float4 color : SV_TARGET0;
	uint2 pick : SV_TARGET1;
};

PSOutputForward PSMain(MeshOutput input)
{
	PSOutputForward output;
	output.color = float4(DiffuseTexture.Sample(Sampler, input.Texcoord).rgb * Cam.albedoColor.rgb, 1.0f);
	output.pick  = uint2(Cam.objectId, Cam.submeshIndex);
	return output;
}

float RenderShadow(float3 viewPos, float3 worldPos)
{
	if (ShadowEnabled == 0u)
		return 1.0;

	float viewDepth = abs(viewPos.z);
	uint cascadeIdx = Shadow_SelectCascade(viewDepth, CascadeSplits, CascadeCount);

	float lit0 = Shadow_SampleCascadeForward(
	    ShadowMapTexture_0,
	    ShadowMapTexture_1,
	    ShadowMapTexture_2,
	    ShadowMapTexture_3,
	    ShadowMapSampler,
	    cascadeIdx,
	    CascadeVP[cascadeIdx],
	    worldPos);

	uint cc = max(min(CascadeCount, 4u), 1u);
	if (cascadeIdx + 1u < cc)
	{
		float splitEnd = CascadeSplits[cascadeIdx];
		float band = splitEnd * 0.1;
		float blend = saturate((viewDepth - (splitEnd - band)) / max(band, 1e-4));
		if (blend > 0.0)
		{
			uint nextCascade = cascadeIdx + 1u;
			float lit1 = Shadow_SampleCascadeForward(
			    ShadowMapTexture_0,
			    ShadowMapTexture_1,
			    ShadowMapTexture_2,
			    ShadowMapTexture_3,
			    ShadowMapSampler,
			    nextCascade,
			    CascadeVP[nextCascade],
			    worldPos);
			lit0 = lerp(lit0, lit1, blend);
		}
	}
	return lit0;
}

PSOutputForward PSMainPhong(MeshOutput input)
{
	float3 albedo = DiffuseTexture.Sample(Sampler, input.Texcoord).rgb * Cam.albedoColor.rgb;
	float3 N = normalize(input.Normal);
	float3 L = normalize(LightDirection.xyz);
	float3 V = normalize(CameraWorldPos.xyz - input.Pos);

	float shadow = RenderShadow(input.ViewPos, input.Pos);

	float3 rgb = Lighting_BlinnPhongLit(albedo, N, L, V, LightRadiance.rgb, shadow, kMeshletAmbient);

#ifdef IMMORTAL_CSM_DEBUG	
    float viewDepth = abs(input.ViewPos.z);
    uint cascadeIdx = Shadow_SelectCascade(viewDepth, CascadeSplits, CascadeCount);
    switch (cascadeIdx)
    {
		case 0 :
			rgb *= float3(1.0f, 0.25f, 0.25f);
			break;
		case 1 :
			rgb *= float3(0.25f, 1.0f, 0.25f);
			break;
		case 2 :
			rgb *= float3(0.25f, 0.25f, 1.0f);
			break;
		case 3 :
			rgb *= float3(1.0f, 1.0f, 0.25f);
			break;
	}
#endif

	PSOutputForward o;
	o.color = float4(rgb, 1.0);
	o.pick  = uint2(Cam.objectId, Cam.submeshIndex);
	return o;
}

PSOutputForward PSMainPBR(MeshOutput input)
{
	float3 albedo = DiffuseTexture.Sample(Sampler, input.Texcoord).rgb * Cam.albedoColor.rgb;
	float3 N      = normalize(input.Normal);
	float3 V      = normalize(CameraWorldPos.xyz - input.Pos);

	float lsh[4] = { 1.0, 1.0, 1.0, 1.0 };
	lsh[0]       = RenderShadow(input.ViewPos, input.Pos);

	LightItem Ls[4];
	Ls[0].Direction  = LightDirection;
	Ls[0].Radiance   = LightRadiance;
	Ls[0].Position   = float4(0.0, 0.0, 0.0, 0.0);
	Ls[0].SpotParams = float4(0.0, 0.0, 0.0, 0.0);

	float3 indirect = Lighting_IndirectIBL(
	    IrradianceEnvMap,
	    PrefilterEnvMap,
	    BRDFLUTMap,
	    IBLClampSampler,
	    albedo,
	    N,
	    V,
	    Cam.roughness,
	    Cam.metallic,
	    UseIBL,
	    IBLMaxSpecularLOD);
	float3 rgb = Lighting_AccumulateLights(
	    Ls,
	    1u,
	    input.Pos,
	    N,
	    V,
	    albedo,
	    Cam.roughness,
	    Cam.metallic,
	    indirect,
	    lsh);
	rgb += max(Cam.emissive.rgb, float3(0, 0, 0));

	PSOutputForward o;
	o.color = float4(rgb, 1.0);
	o.pick  = uint2(Cam.objectId, Cam.submeshIndex);
	return o;
}

PSOutputForward PSMainNPR(MeshOutput input)
{
	float3 albedo = DiffuseTexture.Sample(Sampler, input.Texcoord).rgb * Cam.albedoColor.rgb;
	float3 N = normalize(input.Normal);
	float3 L = normalize(LightDirection.xyz);
	float ndl = dot(N, L);
	
    float shadow = RenderShadow(input.ViewPos, input.Pos);
	
	float3 lit = LightRadiance.rgb * albedo * shadow;
	float3 sh = albedo * kMeshletAmbient * shadow;
	float3 rgb = Lighting_CelDiffuse(ndl, lit, sh, 4.0);

	PSOutputForward o;
	o.color = float4(rgb, 1.0);
	o.pick  = uint2(Cam.objectId, Cam.submeshIndex);
	return o;
}

struct PSOutputGBuffer
{
	float4 albedo : SV_TARGET0;
	float4 normal : SV_TARGET1;
	uint2 objectPick : SV_TARGET2;
	float4 emissive : SV_TARGET3;
};

PSOutputGBuffer PSMainGBuffer(MeshOutput input)
{
	float3 alb = DiffuseTexture.Sample(Sampler, input.Texcoord).rgb * Cam.albedoColor.rgb;
	float3 n = normalize(input.Normal);
	PSOutputGBuffer o;
	o.albedo     = float4(alb, Cam.metallic);
	o.normal     = float4(n * 0.5 + 0.5, Cam.roughness);
	o.objectPick = uint2(Cam.objectId, Cam.submeshIndex);
	o.emissive   = float4(max(Cam.emissive.rgb, float3(0, 0, 0)), 1.0);
	return o;
}
