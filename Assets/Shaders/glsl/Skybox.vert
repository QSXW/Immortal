#version 450 core

layout(set = 0, binding = 0) uniform TransformUniforms
{
	mat4 viewProjectionMatrix;
	mat4 skyProjectionMatrix;
	mat4 sceneRotationMatrix;
};

layout(location = 0) in vec3 position;
layout(location = 0) out vec3 outUVW;

void main()
{
	outUVW = position;
	outUVW.xy *= -1.0;
	gl_Position = skyProjectionMatrix * vec4(position, 1.0);
}
