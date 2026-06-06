// Diffuse irradiance: Sascha Willems irradiancecube.frag (phi/theta grid on hemisphere).

struct IBLIrradiancePC
{
	float DeltaPhi;
	float DeltaTheta;
	uint OutputSize;
	uint _pad;
};

/* Variable must be named `push_constant` so D3D12 reflection matches root constants (see Shader.cpp). */
[[vk::push_constant]]
IBLIrradiancePC push_constant;

TextureCube<float4> EnvironmentMap : register(t0);
SamplerState LinearClamp : register(s1);
RWTexture2DArray<float4> IrradianceOut : register(u2);

static const float PI = 3.14159265358979323846f;

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

	float3 up = float3(0.0f, 1.0f, 0.0f);
	float3 right = normalize(cross(up, N));
	up = cross(N, right);

	const float TWO_PI = PI * 2.0f;
	const float HALF_PI = PI * 0.5f;

	/* Hemisphere integral: E = ∫∫ L(w) cos θ dω = ∫_φ ∫_θ L(θ,φ) cos θ sin θ dθ dφ (Sascha irradiancecube). */
	float3 color = float3(0.0f, 0.0f, 0.0f);
	float dPhi = max(push_constant.DeltaPhi, 1e-6f);
	float dTheta = max(push_constant.DeltaTheta, 1e-6f);
	for (float phi = 0.0f; phi < TWO_PI; phi += dPhi)
	{
		for (float theta = 0.0f; theta < HALF_PI; theta += dTheta)
		{
			float3 tempVec = cos(phi) * right + sin(phi) * up;
			float3 sampleVector = cos(theta) * N + sin(theta) * tempVec;
			color += EnvironmentMap.SampleLevel(LinearClamp, sampleVector, 0.0f).rgb * cos(theta) * sin(theta) * dPhi * dTheta;
		}
	}

	/* Lambert diffuse uses kD * albedo * (E / π); screen G-buffer path multiplies by albedo without extra /π. */
	float3 irradiance = color / PI;
	IrradianceOut[DTid] = float4(irradiance, 1.0f);
}
