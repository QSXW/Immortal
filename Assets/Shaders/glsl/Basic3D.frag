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
	float exposure;
	float gamma;
} shading;

layout (binding = 2) uniform sampler2D AlbedoMap;

layout (location = 0) out vec4 outColor;
layout (location = 1) out int  outObjectID;

void main()
{
	vec2 uv = vec2(fsInput.TexCoord.x, 1.0f - fsInput.TexCoord.y);
	vec3 color = texture(AlbedoMap, uv).rgb * inModel.color;

	outColor    = vec4(color, 1.0);
	outObjectID = inModel.objectID;
}
