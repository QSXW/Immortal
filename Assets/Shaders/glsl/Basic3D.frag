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
    vec3 BiTangent;
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

vec3 CalculateNormal(vec2 uv)
{
	vec3 tangentNormal = texture(NormalMap, uv).xyz * 2.0 - 1.0;

	vec3 N = normalize(fsInput.Normal);
	vec3 T = normalize(fsInput.Tangent);
	vec3 B = normalize(cross(N, T));
	mat3 TBN = mat3(T, B, N);
	return normalize(TBN * tangentNormal);
}

void main()
{
	vec2 uv = vec2(fsInput.TexCoord.x, 1.0f - fsInput.TexCoord.y);
	vec3 objectColor = texture(AlbedoMap, uv).rgb * inModel.color;

    float ambientStrength  = 0.1f;
    float specularStrength = 0.5f;

    // Norm
    vec3 N = CalculateNormal(uv);

    // View Direction
    vec3 V = normalize(shading.camPos - fsInput.WorldPos);

    vec3 Lo = vec3(0.0);
	for (int i = 0; i < shading.lights.length(); i++) {
        // Light Direction
        vec3 L = normalize(shading.lights[i].position - fsInput.WorldPos);

        // Reflect Direction
        vec3 R = reflect(-L, N);

        // Ambient
        vec3 ambient = ambientStrength * shading.lights[i].radiance;

        // Diffuse
        float diff  = max(dot(N, L), 0.0);
        vec3 diffuse = diff * shading.lights[i].radiance;

        // Specular
        float spec = pow(max(dot(V, R), 0.0), 32);
        vec3 specular = specularStrength * spec * shading.lights[i].radiance;

        Lo += (ambient + diffuse + specular) * objectColor;
	};

	outColor    = vec4(Lo, 1.0);
	outObjectID = inModel.objectID;
}
