#version 450

layout(location = 0) in vec3 inPos;

layout(location = 0) out vec4 outColor;
layout(location = 1) out flat int  objectID;

layout (binding = 0) uniform Transform
{
	mat4 viewProjection;
} ubo;

layout (push_constant) uniform Model
{
    mat4 transform;
    vec4 color;
    int  objectID;
} model;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main()
{
	gl_Position = ubo.viewProjection * model.transform * vec4(inPos, 1.0);
    outColor    = model.color;

#if VULKAN
	gl_Position.y = -gl_Position.y;
#endif
}
