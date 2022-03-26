#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inColor;

layout (location = 0) out vec3 outColor;

layout (set = 0, binding = 0) uniform UBO
{
	mat4 viewProjection;
	mat4 modelTransform;
} ubo;

void main()
{
	outColor = inColor;
	gl_Position = ubo.viewProjection * ubo.modelTransform * vec4(inPos.xyz, 1.0);
}
