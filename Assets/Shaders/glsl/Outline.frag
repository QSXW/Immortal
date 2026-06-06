#version 450

layout(location = 0) in vec4 inColor;
layout(location = 1) in flat int inObjectID;

layout(location = 0) out vec4 outColor;
layout(location = 1) out int  outObjectID;

void main()
{
	outColor = inColor;
    outObjectID = inObjectID;
}
