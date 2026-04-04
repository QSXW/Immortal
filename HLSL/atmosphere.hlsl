struct VSInput
{
	float3 Pos : POSITION0;
};

struct PushConstant
{
	float4x4 projection;
	float4 exposureGammaQualityPad;
	float4 sunDirIntensity;
};

[[vk::push_constant]] PushConstant pushConstant;

struct PSInput
{
	float4 Pos : SV_POSITION;
	float3 UVW : TEXCOORD0;
};

Texture2D DummyBind : register(t0);
SamplerState LinearClamp : register(s1);

PSInput VSMain(VSInput input)
{
	PSInput o = (PSInput)0;
	o.UVW = input.Pos;
	o.Pos = mul(pushConstant.projection, float4(input.Pos.xyz, 1.0));
	return o;
}

static const float PI = 3.14159265358979323846;

static const float kPlanetRadius = 1.0;
static const float kAtmosphereRadius = 6471000.0 / 6371000.0;
static const float3 kRayOrigin = float3(0.0, 1.0, 0.0);

static const float3 kRlh = float3(5.5e-6, 13.0e-6, 22.4e-6) * 6371000.0;
static const float kMie = 21e-6 * 6371000.0;
static const float kShRlh = 8000.0 / 6371000.0;
static const float kShMie = 1200.0 / 6371000.0;
static const float kMieG = 0.758;

float2 rsi(float3 r0, float3 rd, float sr)
{
	float a = dot(rd, rd);
	float b = 2.0 * dot(rd, r0);
	float c = dot(r0, r0) - sr * sr;
	float d = b * b - 4.0 * a * c;
	if (d < 0.0)
	{
		return float2(1e5, -1e5);
	}
	float sd = sqrt(d);
	float inv = 0.5 / a;
	return float2((-b - sd) * inv, (-b + sd) * inv);
}

// Always-visible day sky: zenith blue, horizon warm, below-horizon dark blue (not black).
float3 DaySkyGradient(float3 r, float3 sunDir, float dayPhase)
{
	r = normalize(r);
	sunDir = normalize(sunDir);
	float y = r.y;
	float3 zen = float3(0.12, 0.38, 0.82);
	float3 hor = float3(0.48, 0.62, 0.92);
	float3 ground = float3(0.05, 0.06, 0.1);

	float up = saturate(y);
	up = pow(up, 0.38);
	float3 upper = lerp(hor, zen, up);

	float dn = saturate(-y);
	dn = pow(dn, 0.48);
	float3 lower = lerp(hor, ground, dn);

	float3 base = (y >= 0.0) ? upper : lower;

	float band = exp(-abs(y) * abs(y) * 55.0);
	float anim = 0.5 + 0.5 * sin(dayPhase * 0.35);
	base += hor * (0.06 * band) * (0.65 + 0.35 * anim);

	float towardSun = saturate(dot(r, sunDir));
	base += float3(1.0, 0.95, 0.85) * (0.045 * towardSun * towardSun * saturate(sunDir.y + 0.35));

	return saturate(base);
}

float3 AtmosphereWwwtyro(float3 r, float3 pSun, float iSun, int iSteps, int jSteps)
{
	pSun = normalize(pSun);
	r = normalize(r);

	float3 r0 = kRayOrigin;

	float2 pAtm = rsi(r0, r, kAtmosphereRadius);
	float tNear = min(pAtm.x, pAtm.y);
	float tFar = max(pAtm.x, pAtm.y);
	tNear = max(tNear, 0.0);

	if (tNear >= tFar - 1e-6)
	{
		return float3(0.0, 0.0, 0.0);
	}

	float2 pPl = rsi(r0, r, kPlanetRadius);
	if (pPl.x > 1e-5)
	{
		tFar = min(tFar, pPl.x);
	}

	if (tNear >= tFar - 1e-6)
	{
		return float3(0.0, 0.0, 0.0);
	}

	float seg = tFar - tNear;
	seg = max(seg, 1e-4);

	float iStepSize = seg / float(iSteps);
	float iTime = 0.0;

	float3 totalRlh = 0.0;
	float3 totalMie = 0.0;

	float iOdRlh = 0.0;
	float iOdMie = 0.0;

	float mu = dot(r, pSun);
	float mumu = mu * mu;
	float gg = kMieG * kMieG;
	float pRlh = (3.0 / (16.0 * PI)) * (1.0 + mumu);
	float pMie = (3.0 / (8.0 * PI)) * ((1.0 - gg) * (mumu + 1.0)) /
	             (pow(max(1e-5, 1.0 + gg - 2.0 * mu * kMieG), 1.5) * (2.0 + gg));

	for (int i = 0; i < 16; i++)
	{
		if (i >= iSteps)
		{
			break;
		}

		float3 iPos = r0 + r * (iTime + iStepSize * 0.5);
		float iHeight = length(iPos) - kPlanetRadius;

		float odStepRlh = exp(-iHeight / kShRlh) * iStepSize;
		float odStepMie = exp(-iHeight / kShMie) * iStepSize;

		iOdRlh += odStepRlh;
		iOdMie += odStepMie;

		float2 jP = rsi(iPos, pSun, kAtmosphereRadius);
		float jStepSize = max(jP.y, 1e-6) / float(jSteps);
		float jTime = 0.0;

		float jOdRlh = 0.0;
		float jOdMie = 0.0;

		for (int j = 0; j < 8; j++)
		{
			if (j >= jSteps)
			{
				break;
			}

			float3 jPos = iPos + pSun * (jTime + jStepSize * 0.5);
			float jHeight = length(jPos) - kPlanetRadius;

			jOdRlh += exp(-jHeight / kShRlh) * jStepSize;
			jOdMie += exp(-jHeight / kShMie) * jStepSize;

			jTime += jStepSize;
		}

		float3 attn = exp(-(kMie * (iOdMie + jOdMie) + kRlh * (iOdRlh + jOdRlh)));

		totalRlh += odStepRlh * attn;
		totalMie += odStepMie * attn;

		iTime += iStepSize;
	}

	return iSun * (pRlh * kRlh * totalRlh + pMie * kMie * totalMie);
}

struct PSOutput
{
	float4 color : SV_TARGET0;
	uint2 pick : SV_TARGET1;
};

PSOutput PSMain(PSInput input)
{
	PSOutput o = (PSOutput)0;

	float exposure = pushConstant.exposureGammaQualityPad.x;
	float gamma = pushConstant.exposureGammaQualityPad.y;
	float quality = pushConstant.exposureGammaQualityPad.z;
	float dayPhase = pushConstant.exposureGammaQualityPad.w;
	float3 sunDir = pushConstant.sunDirIntensity.xyz;
	float sunIntensity = pushConstant.sunDirIntensity.w;

	int iSteps = (quality > 0.5) ? 16 : 8;
	int jSteps = (quality > 0.5) ? 8 : 4;

	float3 r = normalize(input.UVW);
	float3 grad = DaySkyGradient(r, sunDir, dayPhase);
	float3 phys = AtmosphereWwwtyro(r, sunDir, sunIntensity, iSteps, jSteps);

	// Gradient only fills gaps; physical scattering carries most contrast / “texture”.
	float3 c = max(phys, grad * 0.36);
	c = lerp(grad, c, saturate(length(phys) * 2.0 + 0.22));
	c = max(c, grad * 0.16);

	c += DummyBind.SampleLevel(LinearClamp, float2(0.5, 0.5), 0).rgb * 1e-6;

	c *= 1.38;
	float ev = max(exposure, 0.08);
	c = 1.0 - exp(-c * ev);
	c = pow(c, float3(1.0 / gamma, 1.0 / gamma, 1.0 / gamma));

	o.color = float4(c, 1.0);
	o.pick = uint2(0, 0);
	return o;
}
