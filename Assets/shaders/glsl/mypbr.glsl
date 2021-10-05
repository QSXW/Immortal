// Physically Based Rendering
// Copyright (c) 2017-2018 Michał Siejak

#type vertex
#version 450 core

layout(location=0) in vec3 position;
layout(location=1) in vec3 normal;
layout(location=2) in vec3 tangent;
layout(location=3) in vec3 bitangent;
layout(location=4) in vec2 texcoord;

#if VULKAN
layout(set=0, binding=0) uniform TransformUniforms
#else
layout(std140, binding=0) uniform TransformUniforms
#endif // VULKAN
{
	mat4 viewProjectionMatrix;
	mat4 skyProjectionMatrix;
	mat4 sceneRotationMatrix;
};

out vec3 Normal;
out vec3 FragPos;
out vec2 Texcoord;

void main()
{
	Texcoord = vec2(texcoord.x, 1 - texcoord.y);

	gl_Position = viewProjectionMatrix * sceneRotationMatrix * vec4(position, 1.0f);
    FragPos = vec3(sceneRotationMatrix * vec4(position, 1.0f));
    Normal = vec3(normal);
}

#type fragment
#version 450 core

struct AnalyticalLight {
	vec3 direction;
	vec3 radiance;
};

layout(location=0) in Vertex
{
	vec3 position;
	vec2 texcoord;
	mat3 tangentBasis;
} vin;

layout(location=0) out vec4 color;

layout(binding=0) uniform sampler2D u_ObjectColorTexture;

in vec3 FragPos;  
in vec3 Normal;  
in vec2 Texcoord;

uniform vec3 lightPos; 
uniform vec3 viewPos;
uniform vec3 lightColor;

void main()
{
	// Sample input textures to get shading model params.
	vec3 originalObjectColor = texture(u_ObjectColorTexture, Texcoord).rgb;

    // Ambient
    float ambientStrength = 0.1f;
    vec3 ambient = ambientStrength * lightColor;
  	
    // Diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // Specular
    float specularStrength = 0.5f;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;  
        
    vec3 result = (ambient + diffuse + specular) * originalObjectColor;
    color = vec4(result, 1.0f);
}