#version 450

struct Matrial
{
    vec3  color;
    float roughness;
	float metallic;
};

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inTagent;
layout (location = 3) in vec3 inBitagent;
layout (location = 4) in vec2 inTexCoord;

layout (location = 0) out vec3 outWorldPos;
layout (location = 1) out vec3 outNormal;
layout (location = 2) out Matrial outMatrial;

layout (binding = 0) uniform Transform
{
	mat4 viewProjection;
    mat4 skyProjection;
    mat4 sceneRotation;
} ubo;

layout (binding = 2) uniform Model
{
    mat4  transform;
    vec3  color;
    float roughness;
	float metallic;
} model;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main()
{
    outMatrial.color     = model.color;
    outMatrial.roughness = model.roughness;
    outMatrial.metallic  = model.metallic;

	gl_Position =  ubo.viewProjection * model.transform * vec4(inPos, 1.0);

#if VULKAN
	gl_Position.y = -gl_Position.y;
#endif
    outWorldPos = vec3(gl_Position);
    outNormal = mat3(model.transform) * inNormal;
}
