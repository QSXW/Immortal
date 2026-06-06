// Fullscreen post-process: composites a colored outline around the selected entity
// (or a single sub-mesh) by edge-detecting the object-ID buffer.
// ObjectIdMap is R32G32_UINT: .r = entityId, .g = submeshIndex.

#ifdef __spirv__
#define DEFINE_AS_PUSH_CONSTANT [[vk::push_constant]]
#else
#define DEFINE_AS_PUSH_CONSTANT
#endif

static const uint SUBMESH_ALL = 0xFFFFFFFF;

struct OutlineParams
{
	uint   SelectedId;
	uint   SelectedSubMesh;
	float  Thickness;
	float  _pad;
	float4 Color;
};

DEFINE_AS_PUSH_CONSTANT OutlineParams Params;

Texture2D        SceneColor  : register(t0);
Texture2D<uint2> ObjectIdMap : register(t1);
SamplerState     LinearClamp : register(s2);

struct PSInput
{
	float4 pos : SV_POSITION;
	float2 uv  : TEXCOORD0;
};

PSInput VSMain(uint vertexId : SV_VertexID)
{
	float2 uv = float2((vertexId << 1) & 2, vertexId & 2);
	PSInput o;
	o.pos = float4(uv * float2(2.0, -2.0) + float2(-1.0, 1.0), 0.0, 1.0);
	o.uv  = uv;
	return o;
}

bool IsSelected(uint2 id)
{
	if (id.r != Params.SelectedId)
		return false;
	if (Params.SelectedSubMesh == SUBMESH_ALL)
		return true;
	return id.g == Params.SelectedSubMesh;
}

float4 PSMain(PSInput input) : SV_TARGET
{
	float4 color = SceneColor.Sample(LinearClamp, input.uv);

	if (Params.SelectedId == 0)
	{
		return color;
	}

	int2 pos    = int2(input.pos.xy);
	bool isSel  = IsSelected(ObjectIdMap.Load(int3(pos, 0)));
	bool isEdge = false;
	int  radius = max((int)Params.Thickness, 1);

	for (int i = -radius; i <= radius && !isEdge; i++)
	{
		if (i == 0)
			continue;
		bool hSel = IsSelected(ObjectIdMap.Load(int3(pos + int2(i, 0), 0)));
		bool vSel = IsSelected(ObjectIdMap.Load(int3(pos + int2(0, i), 0)));
		if (isSel != hSel || isSel != vSel)
		{
			isEdge = true;
		}
	}

	if (isEdge)
	{
		color.rgb = Params.Color.rgb;
	}

	return color;
}
