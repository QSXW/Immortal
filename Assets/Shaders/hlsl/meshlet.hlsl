#ifdef __spirv__
#define DEFINE_AS_PUSH_CONSTANT [[vk::push_constant]]
#else
#define DEFINE_AS_PUSH_CONSTANT
#endif

struct CameraProperties
{
    float4x4 MVP;
    uint objectId;
};

DEFINE_AS_PUSH_CONSTANT CameraProperties Cam;

// DEFINE_AS_PUSH_CONSTANT
// ConstantBuffer<CameraProperties> Cam : register(b0);

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

struct MeshOutput {
    float4 Position : SV_POSITION;
    float3 Color    : COLOR;
    float2 Texcoord : TEXCOORD;
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

    if (gtid < m.TriangleCount) {
        //
        // meshopt stores the triangle offset in bytes since it stores the
        // triangle indices as 3 consecutive bytes.
        //
        // Since we repacked those 3 bytes to a 32-bit uint, our offset is now
        // aligned to 4 and we can easily grab it as a uint without any
        // additional offset math.
        //
        uint packed = TriangleIndices[m.TriangleOffset + gtid];
        uint vIdx0  = (packed >>  0) & 0xFF;
        uint vIdx1  = (packed >>  8) & 0xFF;
        uint vIdx2  = (packed >> 16) & 0xFF;
        triangles[gtid] = uint3(vIdx0, vIdx1, vIdx2);
    }

    if (gtid < m.VertexCount) {
        uint vertexIndex = m.VertexOffset + gtid;
        vertexIndex = VertexIndices[vertexIndex];

        vertices[gtid].Position = mul(Cam.MVP, float4(Vertices[vertexIndex].Position, 1.0f));

        float3 color = float3(
            float(gid & 1),
            float(gid & 3) / 4,
            float(gid & 7) / 8);
        vertices[gtid].Color = color;
        vertices[gtid].Texcoord = Vertices[vertexIndex].Texcoord;
    }
}

struct PSOutput
{
    float4 color;
    uint id;
};

PSOutput PSMain(MeshOutput input) : SV_TARGET
{
    PSOutput output;
    output.color = float4(input.Color, 1);
    output.color = float4(DiffuseTexture.Sample(Sampler, input.Texcoord).rgb, 1.0f);
    output.id    = Cam.objectId;
    return output;
}
