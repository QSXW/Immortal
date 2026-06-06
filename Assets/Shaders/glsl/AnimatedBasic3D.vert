#version 450 core

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
    vec2 TexCoord;
};

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inTangent;
layout (location = 3) in vec2 inTexCoord;
layout (location = 4) in ivec4 inBoneIDs;
layout (location = 5) in vec4  inWeights;

layout (location = 0) out FSInput fsInput;
layout (location = 4) out flat ModelInfo outModel;

layout (binding = 0) uniform Transform
{
	mat4 viewProjection;
    mat4 skyProjection;
    mat4 sceneRotation;
} ubo;

#define MAX_BONES 100
layout (binding = 7) uniform Bone
{
    mat4 transforms[MAX_BONES];
} bone;

layout (push_constant) uniform Model
{
    mat4  transform;
    vec3  color;
    float roughness;
	float metallic;
    int   objectID;
} model;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main()
{
    outModel.color     = model.color;
    outModel.roughness = model.roughness;
    outModel.metallic  = model.metallic;
    outModel.objectID  = model.objectID;

    mat4 boneTransform = bone.transforms[inBoneIDs[0]] * inWeights[0];
    boneTransform += bone.transforms[inBoneIDs[1]] * inWeights[1];
    boneTransform += bone.transforms[inBoneIDs[2]] * inWeights[2];
    boneTransform += bone.transforms[inBoneIDs[3]] * inWeights[3];

    mat4 transform = boneTransform;
    vec4 worldPos = boneTransform * vec4(inPos, 1.0);
	gl_Position =  ubo.viewProjection * worldPos;

    fsInput.WorldPos  = vec3(worldPos);
    fsInput.Normal    = mat3(boneTransform) * inNormal;
    fsInput.Tangent   = inTangent;
    fsInput.TexCoord  = inTexCoord;

#if VULKAN
	gl_Position.y = -gl_Position.y;
#endif
}
