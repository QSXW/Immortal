// Diffuse irradiance: cosine-weighted hemisphere integration of radiance cubemap (one mip).

struct IBLIrradiancePC
{
	uint FaceSize;
	uint SampleCount;
	uint2 _pad;
};

[[vk::push_constant]] IBLIrradiancePC pc;

TextureCube<float4> EnvironmentMap : register(t0);
SamplerState LinearClamp : register(s1);
RWTexture2DArray<float4> IrradianceOut : register(u2);

static const float PI = 3.14159265358979323846;

float3 FaceUVToDirection(uint face, float2 uv)
{
	float2 st = uv * 2.0f - 1.0f;
	if (face == 0u)
		return normalize(float3(1.0f, -st.y, -st.x));
	if (face == 1u)
		return normalize(float3(-1.0f, -st.y, st.x));
	if (face == 2u)
		return normalize(float3(st.x, 1.0f, st.y));
	if (face == 3u)
		return normalize(float3(st.x, -1.0f, -st.y));
	if (face == 4u)
		return normalize(float3(st.x, -st.y, 1.0f));
	return normalize(float3(-st.x, -st.y, -1.0f));
}

void BuildBasis(float3 N, out float3 T, out float3 B)
{
	float3 up = abs(N.z) < 0.999f ? float3(0.0f, 0.0f, 1.0f) : float3(1.0f, 0.0f, 0.0f);
	T = normalize(cross(up, N));
	B = cross(N, T);
}

[numthreads(16, 16, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	if (DTid.x >= pc.FaceSize || DTid.y >= pc.FaceSize)
	{
		return;
	}
	uint face = DTid.z;
	if (face >= 6u)
	{
		return;
	}

	float2 uv = (float2(DTid.xy) + 0.5f) / float(pc.FaceSize);
	float3 N = FaceUVToDirection(face, uv);

	float3 T, B;
	BuildBasis(N, T, B);

	uint sampleCount = max(pc.SampleCount, 1u);
	float3 acc = 0.0f;
	for (uint i = 0u; i < sampleCount; i++)
	{
		float a = float(i + 1u) * 0.6180339887f;
		float b = float(i + 1u) * 0.3819660113f;
		float u = frac(a);
		float v = frac(b);
		float phi = 2.0f * PI * u;
		float cosTheta = sqrt(1.0f - v);
		float sinTheta = sqrt(v);
		float3 Lloc = float3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
		float3 L = normalize(T * Lloc.x + B * Lloc.y + N * Lloc.z);
		float3 radiance = EnvironmentMap.SampleLevel(LinearClamp, L, 0.0f).rgb;
		acc += radiance;
	}

	float3 irradiance = acc / float(sampleCount);
	IrradianceOut[DTid] = float4(irradiance, 1.0f);
}
