// Specular environment prefilter: based on Sascha Willems / Epic PBR (GGX importance sampling + PDF mip bias).

struct IBLPrefilterPC
{
	float Roughness;
	uint NumSamples;
	uint OutputSize;
	uint _pad;
};

[[vk::push_constant]]
IBLPrefilterPC push_constant;

TextureCube<float4> EnvironmentMap : register(t0);
SamplerState LinearClamp : register(s1);
RWTexture2DArray<float4> PrefilterOut : register(u2);

static const float PI = 3.1415926536f;

float random(float2 co)
{
	float a = 12.9898;
	float b = 78.233;
	float c = 43758.5453;
	float dt = dot(co.xy, float2(a, b));
	float sn = fmod(dt, 3.14);
	return frac(sin(sn) * c);
}

float2 hammersley2d(uint i, uint N)
{
	uint bits = (i << 16u) | (i >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	float rdi = float(bits) * 2.3283064365386963e-10f;
	return float2(float(i) / float(N), rdi);
}

float3 importanceSample_GGX(float2 Xi, float roughness, float3 normal)
{
	float alpha = roughness * roughness;
	float phi = 2.0f * PI * Xi.x + random(normal.xz) * 0.1f;
	float cosTheta = sqrt((1.0f - Xi.y) / (1.0f + (alpha * alpha - 1.0f) * Xi.y));
	float sinTheta = sqrt(1.0f - cosTheta * cosTheta);
	float3 H = float3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);

	float3 up = abs(normal.z) < 0.999f ? float3(0.0f, 0.0f, 1.0f) : float3(1.0f, 0.0f, 0.0f);
	float3 tangentX = normalize(cross(up, normal));
	float3 tangentY = normalize(cross(normal, tangentX));

	return normalize(tangentX * H.x + tangentY * H.y + normal * H.z);
}

float D_GGX(float dotNH, float roughness)
{
	float alpha = roughness * roughness;
	float alpha2 = alpha * alpha;
	float denom = dotNH * dotNH * (alpha2 - 1.0f) + 1.0f;
	return (alpha2) / (PI * denom * denom);
}

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

float3 prefilterEnvMap(float3 R, float roughness, uint numSamples)
{
	float3 N = R;
	float3 V = R;
	float3 color = float3(0.0f, 0.0f, 0.0f);
	float totalWeight = 0.0f;
	uint envW = 0u;
	uint envH = 0u;
	uint envLevels = 0u;
	EnvironmentMap.GetDimensions(0u, envW, envH, envLevels);
	float envMapDim = float(envW);

	uint Ns = max(numSamples, 1u);
	for (uint i = 0u; i < Ns; i++)
	{
		float2 Xi = hammersley2d(i, Ns);
		float3 H = importanceSample_GGX(Xi, roughness, N);
		float3 L = 2.0f * dot(V, H) * H - V;
		float dotNL = clamp(dot(N, L), 0.0f, 1.0f);
		if (dotNL > 0.0f)
		{
			float dotNH = clamp(dot(N, H), 0.0f, 1.0f);
			float dotVH = clamp(dot(V, H), 0.0f, 1.0f);

			float pdf = D_GGX(dotNH, roughness) * dotNH / (4.0f * dotVH) + 0.0001f;
			float omegaS = 1.0f / (float(Ns) * pdf);
			float omegaP = 4.0f * PI / (6.0f * envMapDim * envMapDim);
			float mipLevel = roughness == 0.0f ? 0.0f : max(0.5f * log2(omegaS / omegaP) + 1.0f, 0.0f);
			color += EnvironmentMap.SampleLevel(LinearClamp, L, mipLevel).rgb * dotNL;
			totalWeight += dotNL;
		}
	}
	return totalWeight > 0.0f ? (color / totalWeight) : color;
}

[numthreads(16, 16, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	uint w = push_constant.OutputSize;
	uint h = push_constant.OutputSize;
	if (DTid.x >= w || DTid.y >= h)
	{
		return;
	}
	uint face = DTid.z;
	if (face >= 6u)
	{
		return;
	}

	float2 uv = (float2(DTid.xy) + 0.5f) / float2(w, h);
	float3 N = FaceUVToDirection(face, uv);
	float3 c = prefilterEnvMap(N, push_constant.Roughness, push_constant.NumSamples);
	PrefilterOut[DTid] = float4(c, 1.0f);
}
