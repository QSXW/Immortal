/* Directional shadow depth pass: mesh + empty PS. Bindings t0–t3 only (no albedo / sampler). */

#ifdef __spirv__
#define DEFINE_AS_PUSH_CONSTANT [[vk::push_constant]]
#else
#define DEFINE_AS_PUSH_CONSTANT
#endif

struct CameraProperties
{
	float4x4 MVP;
	float4x4 Model;
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

struct MeshOutput
{
	float4 Position : SV_POSITION;
};

struct PSOutputShadowDepthRgb
{
	float4 DepthRgb : SV_Target0;
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
	}
}

PSOutputShadowDepthRgb PSMain(MeshOutput input)
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
