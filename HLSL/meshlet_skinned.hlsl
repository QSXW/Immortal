#ifdef __spirv__
#define DEFINE_AS_PUSH_CONSTANT [[vk::push_constant]]
#else
#define DEFINE_AS_PUSH_CONSTANT
#endif

struct CameraProperties
{
	float4x4 MVP;
	float4x4 Model;
	uint objectId;
	uint submeshIndex;
	float roughness;
	float metallic;
	float4 albedoColor;
	float4 emissive;
};

DEFINE_AS_PUSH_CONSTANT CameraProperties Cam;

struct SkinnedVertex
{
	float3 Position;
	float3 Normal;
	float3 Tangent;
	float2 Texcoord;
	uint4 BoneIds;
	float4 Weights;
};

struct Meshlet
{
	uint VertexOffset;
	uint TriangleOffset;
	uint VertexCount;
	uint TriangleCount;
};

StructuredBuffer<SkinnedVertex> Vertices        : register(t0);
StructuredBuffer<Meshlet>       Meshlets        : register(t1);
StructuredBuffer<uint>          VertexIndices   : register(t2);
StructuredBuffer<uint>          TriangleIndices : register(t3);
Texture2D                       DiffuseTexture  : register(t4);
SamplerState                    Sampler         : register(s5);
StructuredBuffer<float4x4>      BoneMatrices    : register(t6);

struct MeshOutput
{
	float4 Position : SV_POSITION;
	float3 Color    : COLOR;
	float2 Texcoord : TEXCOORD;
	float3 Normal   : NORMAL;
};

float4x4 BuildSkinMatrix(SkinnedVertex v)
{
	float4x4 b0 = BoneMatrices[v.BoneIds.x];
	float4x4 b1 = BoneMatrices[v.BoneIds.y];
	float4x4 b2 = BoneMatrices[v.BoneIds.z];
	float4x4 b3 = BoneMatrices[v.BoneIds.w];
	return b0 * v.Weights.x + b1 * v.Weights.y + b2 * v.Weights.z + b3 * v.Weights.w;
}

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
		uint vIdx0  = (packed >>  0) & 0xFF;
		uint vIdx1  = (packed >>  8) & 0xFF;
		uint vIdx2  = (packed >> 16) & 0xFF;
		triangles[gtid] = uint3(vIdx0, vIdx1, vIdx2);
	}

	if (gtid < m.VertexCount)
	{
		uint vertexIndex = m.VertexOffset + gtid;
		vertexIndex = VertexIndices[vertexIndex];

		SkinnedVertex v = Vertices[vertexIndex];
		float4x4 skin = BuildSkinMatrix(v);
		float4 modelPos = mul(skin, float4(v.Position, 1.0f));
		vertices[gtid].Position = mul(Cam.MVP, modelPos);

		float3 color = float3(
		    float(gid & 1),
		    float(gid & 3) / 4,
		    float(gid & 7) / 8);
		vertices[gtid].Color = color;
		vertices[gtid].Texcoord = v.Texcoord;

		float3x3 skin3 = (float3x3)skin;
		float3 n = mul((float3x3)Cam.Model, mul(skin3, v.Normal));
		vertices[gtid].Normal = normalize(n);
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
	float3 tex = DiffuseTexture.Sample(Sampler, input.Texcoord).rgb;
	output.color = float4(tex * Cam.albedoColor.rgb, Cam.albedoColor.a);
	output.pick  = uint2(Cam.objectId, Cam.submeshIndex);
	return output;
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
	o.albedo = float4(alb, Cam.metallic);
	o.normal = float4(n * 0.5 + 0.5, Cam.roughness);
	o.objectPick = uint2(Cam.objectId, Cam.submeshIndex);
	o.emissive = float4(max(Cam.emissive.rgb, float3(0, 0, 0)), 1.0);
	return o;
}
