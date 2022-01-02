#version 450

layout(location = 0) in vec3  inPosition;
layout(location = 1) in vec4  inColor;
layout(location = 2) in vec2  inTexCoord;
layout(location = 3) in float inTexIndex;
layout(location = 4) in float inTilingFactor;
layout(location = 5) in int   inEntityID;

layout (binding = 0) uniform UBO
{
	mat4 viewProjection;
} ubo;

layout(location = 0) out vec4       outColor;
layout(location = 1) out vec2       outTexCoord;
layout(location = 2) out flat float outTexIndex;
layout(location = 3) out float      outTilingFactor;
layout(location = 4) out flat int   outEntityID;

void main()
{
	outColor        = inColor;
	outTexCoord     = inTexCoord;
	outTexIndex     = inTexIndex;
	outTilingFactor = inTilingFactor;
	outEntityID     = inEntityID;

	gl_Position = ubo.viewProjection * vec4(inPosition, 1.0);
#if VULKAN
	gl_Position.y = -gl_Position.y;
#endif
}
