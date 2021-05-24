// Basic Texture Shader

#type vertex
#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal; 
layout(location = 2) in vec3 tagent;
layout(location = 3) in vec3 bitangen;
layout(location = 4) in vec2 texcoord;

uniform mat4 u_ViewProjection;

void main()
{
	gl_Position = u_ViewProjection * vec4(position, 1.0);
}

#type fragment
#version 450

layout(location = 0) out vec4 color;

void main()
{
	color = vec4(1.0, 1.0, 1.0f, 1.0f);
}