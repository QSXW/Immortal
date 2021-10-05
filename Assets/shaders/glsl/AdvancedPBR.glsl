#type vertex
#version 450 core

layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec3 Tangent;
layout(location = 3) in vec3 Binormal;
layout(location = 4) in vec2 TexCoord;

layout(std140, binding = 0) uniform Camera
{
    mat4 ViewProjectionMatrix;
    mat4 u_InverseViewProjectionMatrix;
}

#type fragment
#version 450 core

struct PhysicallyBasedRenderingParameters
{
    vec3  Albedo;
    float Roughness;
    float Metalness;
    vec3  Normal;
    vec3  View;
    float NdotV;
};

PhysicallyBasedRenderingParameters mData;

void main()
{

}