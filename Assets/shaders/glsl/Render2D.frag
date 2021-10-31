#version 450

layout(location = 0) out vec4 outColor;
layout(location = 1) out int  outID;

layout(location = 0) in vec4       inColor;
layout(location = 1) in vec2       inTexCoord;
layout(location = 2) in flat float inTexIndex;
layout(location = 3) in float      inTilingFactor;
layout(location = 4) in flat int   inEntityID;

layout(binding = 1) uniform sampler2D uTextures[32];

void main()
{
	vec4 texColor = inColor;

	switch(int(inTexIndex))
	{
		case  0: texColor *= texture(uTextures[ 0], inTexCoord * inTilingFactor); break;
		case  1: texColor *= texture(uTextures[ 1], inTexCoord * inTilingFactor); break;
		case  2: texColor *= texture(uTextures[ 2], inTexCoord * inTilingFactor); break;
		case  3: texColor *= texture(uTextures[ 3], inTexCoord * inTilingFactor); break;
		case  4: texColor *= texture(uTextures[ 4], inTexCoord * inTilingFactor); break;
		case  5: texColor *= texture(uTextures[ 5], inTexCoord * inTilingFactor); break;
		case  6: texColor *= texture(uTextures[ 6], inTexCoord * inTilingFactor); break;
		case  7: texColor *= texture(uTextures[ 7], inTexCoord * inTilingFactor); break;
		case  8: texColor *= texture(uTextures[ 8], inTexCoord * inTilingFactor); break;
		case  9: texColor *= texture(uTextures[ 9], inTexCoord * inTilingFactor); break;
		case 10: texColor *= texture(uTextures[10], inTexCoord * inTilingFactor); break;
		case 11: texColor *= texture(uTextures[11], inTexCoord * inTilingFactor); break;
		case 12: texColor *= texture(uTextures[12], inTexCoord * inTilingFactor); break;
		case 13: texColor *= texture(uTextures[13], inTexCoord * inTilingFactor); break;
		case 14: texColor *= texture(uTextures[14], inTexCoord * inTilingFactor); break;
		case 15: texColor *= texture(uTextures[15], inTexCoord * inTilingFactor); break;
		case 16: texColor *= texture(uTextures[16], inTexCoord * inTilingFactor); break;
		case 17: texColor *= texture(uTextures[17], inTexCoord * inTilingFactor); break;
		case 18: texColor *= texture(uTextures[18], inTexCoord * inTilingFactor); break;
		case 19: texColor *= texture(uTextures[19], inTexCoord * inTilingFactor); break;
		case 20: texColor *= texture(uTextures[20], inTexCoord * inTilingFactor); break;
		case 21: texColor *= texture(uTextures[21], inTexCoord * inTilingFactor); break;
		case 22: texColor *= texture(uTextures[22], inTexCoord * inTilingFactor); break;
		case 23: texColor *= texture(uTextures[23], inTexCoord * inTilingFactor); break;
		case 24: texColor *= texture(uTextures[24], inTexCoord * inTilingFactor); break;
		case 25: texColor *= texture(uTextures[25], inTexCoord * inTilingFactor); break;
		case 26: texColor *= texture(uTextures[26], inTexCoord * inTilingFactor); break;
		case 27: texColor *= texture(uTextures[27], inTexCoord * inTilingFactor); break;
		case 28: texColor *= texture(uTextures[28], inTexCoord * inTilingFactor); break;
		case 29: texColor *= texture(uTextures[29], inTexCoord * inTilingFactor); break;
		case 30: texColor *= texture(uTextures[30], inTexCoord * inTilingFactor); break;
		case 31: texColor *= texture(uTextures[31], inTexCoord * inTilingFactor); break;
	}
	outColor = texColor;

	outID = int(inEntityID);
}
