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
	uint _pad0;
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

Texture2D    DiffuseTexture    : register(t4);
SamplerState Sampler           : register(s5);

struct MeshOutput
{
	float4 Position : SV_POSITION;
	float3 Color    : COLOR;
	float2 Texcoord : TEXCOORD;
	float3 Normal   : NORMAL;
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
		uint vIdx0  = (packed >>  0) & 0xFF;
		uint vIdx1  = (packed >>  8) & 0xFF;
		uint vIdx2  = (packed >> 16) & 0xFF;
		triangles[gtid] = uint3(vIdx0, vIdx1, vIdx2);
	}

	if (gtid < m.VertexCount)
	{
		uint vertexIndex = m.VertexOffset + gtid;
		vertexIndex = VertexIndices[vertexIndex];

		vertices[gtid].Position = mul(Cam.MVP, float4(Vertices[vertexIndex].Position, 1.0f));

		float3 color = float3(
		    float(gid & 1),
		    float(gid & 3) / 4,
		    float(gid & 7) / 8);
		vertices[gtid].Color = color;
		vertices[gtid].Texcoord = Vertices[vertexIndex].Texcoord;

		float3 n = mul((float3x3)Cam.Model, Vertices[vertexIndex].Normal);
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
	output.color = float4(DiffuseTexture.Sample(Sampler, input.Texcoord).rgb, 1.0f);
	output.pick  = uint2(Cam.objectId, Cam.submeshIndex);
	return output;
}

struct PSOutputGBuffer
{
	float4 albedo : SV_TARGET0;
	float4 normal : SV_TARGET1;
	uint2 objectPick : SV_TARGET2;
};

PSOutputGBuffer PSMainGBuffer(MeshOutput input)
{
	float3 alb = DiffuseTexture.Sample(Sampler, input.Texcoord).rgb;
	float3 n = normalize(input.Normal);
	PSOutputGBuffer o;
	o.albedo = float4(alb, Cam.metallic);
	o.normal = float4(n * 0.5 + 0.5, Cam.roughness);
	o.objectPick = uint2(Cam.objectId, Cam.submeshIndex);
	return o;
}
