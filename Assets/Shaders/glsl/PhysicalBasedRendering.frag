#version 450

struct ModelInfo
{
    vec3  color;
    float roughness;
	float metallic;
	int   objectID;
};

struct FSInput
{
    vec3 WorldPos;
    vec3 Normal;
    vec3 Tagent;
    vec3 Bitagent;
    vec2 TexCoord;
};

layout (location = 0) in FSInput fsInput;
layout (location = 5) in flat ModelInfo inModel;

struct Light {
    vec3 position;
    vec3 radiance;
};

layout (binding = 1) uniform Shading {
    Light lights[4];
    vec3 camPos;
} shading;

layout (binding = 2) uniform sampler2D AlbedoMap;

layout (location = 0) out vec4 outColor;
layout (location = 1) out int  outObjectID;

const float PI = 3.14159265359;

/* Normal Distribution function */
float D_GGX(float dotNH, float roughness)
{
	float alpha = roughness * roughness;
	float alpha2 = alpha * alpha;
	float denom = dotNH * dotNH * (alpha2 - 1.0) + 1.0;
	return (alpha2)/(PI * denom*denom);
}

/* Geometric Shadowing function */
float G_SchlicksmithGGX(float dotNL, float dotNV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r*r) / 8.0;
	float GL = dotNL / (dotNL * (1.0 - k) + k);
	float GV = dotNV / (dotNV * (1.0 - k) + k);
	return GL * GV;
}

/* Fresnel function */
vec3 F_Schlick(float cosTheta, float metallic)
{
	vec3 F0 = mix(vec3(0.04), inModel.color, metallic); // * material.specular
	vec3 F = F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
	return F;
}

/* Specular BRDF composition */
vec3 BRDF(vec3 L, vec3 V, vec3 N, float metallic, float roughness)
{
	// Precalculate vectors and dot products
	vec3 H = normalize (V + L);
	float dotNV = clamp(dot(N, V), 0.0, 1.0);
	float dotNL = clamp(dot(N, L), 0.0, 1.0);
	float dotLH = clamp(dot(L, H), 0.0, 1.0);
	float dotNH = clamp(dot(N, H), 0.0, 1.0);

	// Light color fixed
	vec3 lightColor = vec3(1.0);

	vec3 color = vec3(0.0);

	if (dotNL > 0.0)
	{
		float rroughness = max(0.05, roughness);
		// D = Normal distribution (Distribution of the microfacets)
		float D = D_GGX(dotNH, roughness);
		// G = Geometric shadowing term (Microfacets shadowing)
		float G = G_SchlicksmithGGX(dotNL, dotNV, rroughness);
		// F = Fresnel factor (Reflectance depending on angle of incidence)
		vec3 F = F_Schlick(dotNV, metallic);

		vec3 spec = D * F * G / (4.0 * dotNL * dotNV);

		color += spec * dotNL * lightColor;
	}

	return color;
}

void main()
{
	vec2 uv = fsInput.TexCoord;
	vec3 color = texture(AlbedoMap, vec2(uv.x, 1.0f - uv.y) ).rgb * inModel.color;

	vec3 N = normalize(fsInput.Normal);
	vec3 V = normalize(shading.camPos - fsInput.WorldPos);

	float roughness = inModel.roughness;

	roughness = max(roughness, step(fract(fsInput.WorldPos.y * 2.02), 0.5));

	// Specular contribution
	vec3 Lo = vec3(0.0);
	for (int i = 0; i < 3; i++) {
		vec3 L = normalize(shading.lights[i].position - fsInput.WorldPos);
		Lo += BRDF(L, V, N, inModel.metallic, roughness);
	};

	// Combine with ambient
	color += Lo;

	// Gamma correct
	// color = pow(color, vec3(0.4545));

	outColor = vec4(color, 1.0);

	outObjectID = inModel.objectID;
}