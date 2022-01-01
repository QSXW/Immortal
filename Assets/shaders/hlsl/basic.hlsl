//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

cbuffer ubo : register(b0)
{
	float4x4 viewProjection;
	float4x4 modelTransform;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

PSInput VSMain(float4 position : POSITION, float4 color : COLOR)
{
    PSInput result;

    result.color = color;
    result.position = position;
    result.position.y = -position.y;
    result.position = mul(viewProjection, mul(modelTransform, result.position));

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return input.color;
}
