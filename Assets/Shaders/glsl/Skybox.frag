#version 450 core

layout(location = 0) in vec3  inUVW;
layout(location = 0) out vec4 color;
layout(location = 1) out int  outObjectID;

layout(set = 0, binding = 1) uniform samplerCube envTexture;

struct Light {
    vec3 position;
    vec3 radiance;
};

layout (binding = 2) uniform Shading {
    Light lights[4];
    vec3 camPos;
	float exposure;
	float gamma;
} shading;

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
	color = texture(envTexture, inUVW);
    color.xyz = Uncharted2Tonemap(color.xyz * shading.exposure);
	color.xyz = color.xyz * (1.0f / Uncharted2Tonemap(vec3(11.2f)));
	// Gamma correction
	color = pow(color, vec4(1.0f / shading.gamma));

    outObjectID = 0;
}
