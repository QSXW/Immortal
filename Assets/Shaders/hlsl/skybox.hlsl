struct VSInput
{
    float3 Pos      : POSITION0;
	// float3 Normal   : NORMAL0;
	// float2 Texcoord : TEXCOORD0;
};

struct PushConstant
{
	float4x4 projection;
	float exposure;
	float gamma;
};

[[vk::push_constant]] PushConstant pushConstant;

struct PSInput
{
	float4 Pos : SV_POSITION;
    float3 UVW : TEXCOORD0;
};

PSInput VSMain(VSInput input)
{
	PSInput output = (PSInput)0;
	output.UVW      = input.Pos;
	// output.UVW.xy  *= -1.0;
	float4 position = float4(input.Pos.xyz, 1.0);
	output.Pos = mul(pushConstant.projection, position);

	return output;
}

TextureCube Texture : register(t0);
SamplerState Sampler : register(s1);

// From http://filmicworlds.com/blog/filmic-tonemapping-operators/
float3 Uncharted2Tonemap(float3 color)
{
	float A = 0.15;
	float B = 0.50;
	float C = 0.10;
	float D = 0.20;
	float E = 0.02;
	float F = 0.30;
	float W = 11.2;
	return ((color*(A*color+C*B)+D*E)/(color*(A*color+B)+D*F))-E/F;
}

struct PSOutput
{
    float4 color : SV_TARGET;
    uint objectID : COLOR;
};

PSOutput PSMain(PSInput input) : SV_TARGET
{
	PSOutput output;
	float3 color = Texture.Sample(Sampler, input.UVW).rgb;
	// float3 color = input.Pos.xyz;
	// float3 color = input.UVW;

	// Tone mapping
	color = Uncharted2Tonemap(color * pushConstant.exposure);
	color = color * (1.0f / Uncharted2Tonemap((11.2f).xxx));
	// Gamma correction
	color = pow(color, (1.0f / pushConstant.gamma).xxx);

	output.color = float4(color, 1.0f);
	output.objectID = 0;
	return output;
}
