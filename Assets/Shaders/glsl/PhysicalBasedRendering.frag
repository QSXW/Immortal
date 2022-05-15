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
    vec3 Tangent;
    vec2 TexCoord;
};

layout (location = 0) in FSInput fsInput;
layout (location = 4) in flat ModelInfo inModel;

struct Light {
    vec3 position;
    vec3 radiance;
};

layout (binding = 1) uniform Shading {
    Light lights[4];
    vec3 camPos;
	float exposure;
	float gamma;
} shading;

layout (binding = 2) uniform sampler2D AlbedoMap;
layout (binding = 3) uniform sampler2D NormalMap;
layout (binding = 4) uniform sampler2D MetallicMap;
layout (binding = 5) uniform sampler2D RoughnessMap;
layout (binding = 6) uniform sampler2D AOMap;

layout (location = 0) out vec4 outColor;
layout (location = 1) out int  outObjectID;

const float PI = 3.14159265359;
#define ALBEDO texture(AlbedoMap, vec2(fsInput.TexCoord.x, 1.0f - fsInput.TexCoord.y) ).rgb * inModel.color

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

// Fresnel function ----------------------------------------------------
vec3 F_Schlick_V(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}
vec3 F_SchlickR(float cosTheta, vec3 F0, float roughness)
{
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 specularContribution(vec3 L, vec3 V, vec3 N, vec3 F0, float metallic, float roughness)
{
	// Precalculate vectors and dot products
	vec3 H = normalize (V + L);
	float dotNH = clamp(dot(N, H), 0.0, 1.0);
	float dotNV = clamp(dot(N, V), 0.0, 1.0);
	float dotNL = clamp(dot(N, L), 0.0, 1.0);

	// Light color fixed
	vec3 lightColor = vec3(1.0);

	vec3 color = vec3(0.0);

	if (dotNL > 0.0) {
		// D = Normal distribution (Distribution of the microfacets)
		float D = D_GGX(dotNH, roughness);
		// G = Geometric shadowing term (Microfacets shadowing)
		float G = G_SchlicksmithGGX(dotNL, dotNV, roughness);
		// F = Fresnel factor (Reflectance depending on angle of incidence)
		vec3 F = F_Schlick_V(dotNV, F0);
		vec3 spec = D * F * G / (4.0 * dotNL * dotNV + 0.001);
		vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);
		color += (kD * ALBEDO / PI + spec) * dotNL;
	}

	return color;
}

vec3 CalculateNormal(vec2 uv)
{
	vec3 tangentNormal = texture(NormalMap, uv).xyz * 2.0 - 1.0;

	vec3 N = normalize(fsInput.Normal);
	vec3 T = normalize(fsInput.Tangent);
	vec3 B = normalize(cross(N, T));
	mat3 TBN = mat3(T, B, N);
	return normalize(TBN * tangentNormal);
}

// From http://filmicgames.com/archives/75
vec3 Uncharted2Tonemap(vec3 x)
{
	float A = 0.15;
	float B = 0.50;
	float C = 0.10;
	float D = 0.20;
	float E = 0.02;
	float F = 0.30;
	return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

void main()
{
	vec2 uv = vec2(fsInput.TexCoord.x, 1.0f - fsInput.TexCoord.y);
	vec3 albedo = texture(AlbedoMap, uv).rgb * inModel.color;

	vec3 N = CalculateNormal(uv);
	vec3 V = normalize(shading.camPos - fsInput.WorldPos);
	vec3 R = reflect(-V, N);

	float metallic = texture(MetallicMap, uv).r * inModel.metallic;
	float roughness = texture(RoughnessMap, uv).r * inModel.roughness;

	vec3 F0 = vec3(0.04);
	F0 = mix(F0, albedo, metallic);

	// Specular contribution
	vec3 Lo = vec3(0.0);
	for (int i = 0; i < shading.lights.length(); i++) {
		vec3 L = normalize(shading.lights[i].position - fsInput.WorldPos);
		Lo += specularContribution(L, V, N, F0, metallic, roughness);
	};

	// Combine with ambient
	vec3 color = Lo;

	color = Uncharted2Tonemap(color * shading.exposure);
	color = color * (1.0f / Uncharted2Tonemap(vec3(11.2f)));
	// Gamma correction
	color = pow(color, vec3(1.0f / shading.gamma));

	outColor    = vec4(color, 1.0);
	outObjectID = inModel.objectID;
}

