/* Skinned mesh shadow depth pass: t0–t3 + bone buffer t6, no albedo / sampler. */

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
StructuredBuffer<float4x4>      BoneMatrices    : register(t6);

struct MeshOutput
{
	float4 Position : SV_POSITION;
};

struct PSOutputShadowDepthRgb
{
	float4 DepthRgb : SV_Target0;
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
	}
}

PSOutputShadowDepthRgb PSMainShadow(MeshOutput input)
{
	float z = input.Position.z;
	float lo = 0.0;
	float hi = 1.0;
	float span = max(hi - lo, 1e-5);
	float t = saturate((z - lo) / span);
	float v = 1.0 - t;
	float delta = saturate(1.0 - z);
	float vLog = saturate(-log10(max(delta, 1e-7)) / 6.0) * step(1e-4, delta);
	v = max(v, vLog * 0.55);
	PSOutputShadowDepthRgb o;
	o.DepthRgb = float4(v.xxx, 1.0);
	return o;
}
