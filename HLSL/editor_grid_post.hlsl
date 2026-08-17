// Editor grid deferred post: scene color + depth. Entry: VSMain / PSMain.

#ifdef __spirv__
#define DEFINE_AS_PUSH_CONSTANT [[vk::push_constant]]
#else
#define DEFINE_AS_PUSH_CONSTANT
#endif

struct GridFrame
{
	float4x4 InvViewProjection;
	float4x4 ViewProjection;
	float4   CameraWorld;
	float4   Config;
};

DEFINE_AS_PUSH_CONSTANT GridFrame G;

Texture2D    SceneColor : register(t0);
Texture2D    DepthMap : register(t1);
SamplerState LinearClamp : register(s2);

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

static float SquareRim(float3 hit, float halfExtent, float rimFade)
{
	float m = max(abs(hit.x), abs(hit.z));
	if (m > halfExtent)
	{
		return 0.0;
	}
	return 1.0 - smoothstep(halfExtent - rimFade, halfExtent, m);
}

static void GridLineMasksAA(float3 hit, float cell, float majorN, float tPlane, out float minorLine, out float majorLine, out float axisLine, out float lineMask)
{
	float2 g = hit.xz / cell;
	float fw = max(max(fwidth(g.x), fwidth(g.y)), 1e-5);

	float ax = abs(frac(g.x - 0.5) - 0.5);
	float ay = abs(frac(g.y - 0.5) - 0.5);
	float lx = smoothstep(0.0, fw * 1.5, ax);
	float ly = smoothstep(0.0, fw * 1.5, ay);
	minorLine = 1.0 - min(lx, ly);

	float2 gM = g / majorN;
	float aMx = abs(frac(gM.x - 0.5) - 0.5);
	float aMy = abs(frac(gM.y - 0.5) - 0.5);
	float fwM = max(fw / majorN, 1e-5);
	float lMx = smoothstep(0.0, fwM * 1.5, aMx);
	float lMy = smoothstep(0.0, fwM * 1.5, aMy);
	majorLine = 1.0 - min(lMx, lMy);

	float axisW = max(cell * 0.08, 1e-4);
	float axisX = 1.0 - smoothstep(0.0, axisW, abs(hit.z));
	float axisZ = 1.0 - smoothstep(0.0, axisW, abs(hit.x));
	axisLine = saturate(max(axisX, axisZ));

	float lineCore = max(minorLine * 0.5, max(majorLine * 0.85, axisLine * 0.95));
	lineMask = saturate(lineCore);
	lineMask *= exp(-saturate(tPlane * 0.0045));
}

float NdcDepthFromWorld(float3 worldPos, float4x4 vp)
{
	float4 c = mul(vp, float4(worldPos, 1.0));
	return c.z / max(abs(c.w), 1e-5);
}

float4 PSMain(PSInput input) : SV_Target
{
	float4 scene = SceneColor.Sample(LinearClamp, input.uv);

	float depth = DepthMap.Sample(LinearClamp, input.uv).r;
	float x = input.uv.x * 2.0 - 1.0;
	float y = 1.0 - input.uv.y * 2.0;

	float4 clipGeom = float4(x, y, depth, 1.0);
	float4 worldGeomH = mul(G.InvViewProjection, clipGeom);
	float3 worldGeom = worldGeomH.xyz / max(worldGeomH.w, 1e-5);

	float3 cam = G.CameraWorld.xyz;

	float4 clipNear = float4(x, y, 0.0, 1.0);
	float4 wNearH = mul(G.InvViewProjection, clipNear);
	float3 wNear = wNearH.xyz / max(wNearH.w, 1e-5);
	float4 clipFar = float4(x, y, 1.0, 1.0);
	float4 wFarH = mul(G.InvViewProjection, clipFar);
	float3 wFar = wFarH.xyz / max(wFarH.w, 1e-5);
	float3 dir = normalize(wFar - wNear);

	float denom = dir.y;
	if (abs(denom) < 1e-6)
	{
		return scene;
	}

	float tPlane = -cam.y / denom;
	if (tPlane <= 0.0)
	{
		return scene;
	}

	float3 hit = cam + dir * tPlane;

	float halfExtent = max(G.Config.z, 1.0);
	float rimFade = max(G.Config.w, 0.5);
	float rim = SquareRim(hit, halfExtent, rimFade);
	if (rim <= 0.0)
	{
		return scene;
	}

	float zGeom = NdcDepthFromWorld(worldGeom, G.ViewProjection);
	float zPlane = NdcDepthFromWorld(hit, G.ViewProjection);
	static const float zOccludeEps = 2e-4;
	if (zPlane > zGeom - zOccludeEps)
	{
		return scene;
	}

	float cell = max(G.Config.x, 1e-3);
	float majorN = max(G.Config.y, 1.0);
	float minorLine, majorLine, axisLine, lineMaskCore;
	GridLineMasksAA(hit, cell, majorN, tPlane, minorLine, majorLine, axisLine, lineMaskCore);
	float lineMask = lineMaskCore * rim;

	float3 minorCol = float3(0.36, 0.36, 0.38);
	float3 majorCol = float3(0.48, 0.48, 0.50);
	float3 axisCol = float3(0.22, 0.22, 0.24);
	float3 lineRgb = lerp(lerp(minorCol, majorCol, saturate(majorLine)), axisCol, saturate(axisLine) * 0.55);

	float a = lineMask * 0.78;
	return float4(lerp(scene.rgb, lineRgb, a), scene.a);
}
