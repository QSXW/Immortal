#version 450

layout(location = 0) out vec4 outColor;
// layout(location = 1) out int  outID;

layout(location = 0) in vec4       inColor;
layout(location = 1) in vec2       inTexCoord;
layout(location = 2) in flat float inTexIndex;
layout(location = 3) in float      inTilingFactor;
layout(location = 4) in flat int   inEntityID;

layout(binding = 1) uniform sampler2D uTextures[32];

void main()
{
	outColor =  texture(uTextures[int(inTexIndex)], vec2(inTexCoord.x, 1.0 - inTexCoord.y) * inTilingFactor) * inColor;

	// outID = int(inEntityID);
}
