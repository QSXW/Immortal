struct VSInput
{
    float4  position     : POSITION;
    float4  color        : COLOR;
    float2  uv           : TEXCOORD;
    float   index        : INDEX;
    float   tilingFactor : TILING_FACTOR;
    int     id           : OBJECT_ID;
};

struct PSInput
{
    float4 position     : SV_POSITION;
    float4 color        : COLOR;
    float2 uv           : TEXCOORD;
    uint   index        : INDEX;
    float  tilingFactor : TILING_FACTOR;
    int    id           : OBJECT_ID;
};

struct PSOutput
{
    float4 color : SV_TARGET;
    int objectID : COLOR;
};

struct PushConstant
{
	float4x4 viewProjection;
};
[[vk::push_constant]] PushConstant pushConstant;

SamplerState g_sampler : register(s0);
Texture2D g_textures[32] : register(t1);

PSInput VSMain(VSInput input)
{
    PSInput result;

    input.position.y    = -input.position.y;
    result.position     = mul(pushConstant.viewProjection, input.position);
    result.color        = input.color;
    result.uv           = input.uv;
    result.index        = input.index;
    result.tilingFactor = input.tilingFactor;
    result.id           = input.id;

    return result;
}

PSOutput PSMain(PSInput input) : SV_TARGET
{
    PSOutput output;

    float4 result;
    switch(input.index)
    {
        case  0: result = g_textures[ 0].Sample(g_sampler, input.uv * input.tilingFactor); break;
        case  1: result = g_textures[ 1].Sample(g_sampler, input.uv * input.tilingFactor); break;
        case  2: result = g_textures[ 2].Sample(g_sampler, input.uv * input.tilingFactor); break;
        case  3: result = g_textures[ 3].Sample(g_sampler, input.uv * input.tilingFactor); break;
        case  4: result = g_textures[ 4].Sample(g_sampler, input.uv * input.tilingFactor); break;
        case  5: result = g_textures[ 5].Sample(g_sampler, input.uv * input.tilingFactor); break;
        case  6: result = g_textures[ 6].Sample(g_sampler, input.uv * input.tilingFactor); break;
        case  7: result = g_textures[ 7].Sample(g_sampler, input.uv * input.tilingFactor); break;
        case  8: result = g_textures[ 8].Sample(g_sampler, input.uv * input.tilingFactor); break;
        case  9: result = g_textures[ 9].Sample(g_sampler, input.uv * input.tilingFactor); break;
        case 10: result = g_textures[10].Sample(g_sampler, input.uv * input.tilingFactor); break;
        case 11: result = g_textures[11].Sample(g_sampler, input.uv * input.tilingFactor); break;
        case 12: result = g_textures[12].Sample(g_sampler, input.uv * input.tilingFactor); break;
        case 13: result = g_textures[13].Sample(g_sampler, input.uv * input.tilingFactor); break;
        case 14: result = g_textures[14].Sample(g_sampler, input.uv * input.tilingFactor); break;
        case 15: result = g_textures[15].Sample(g_sampler, input.uv * input.tilingFactor); break;
        case 16: result = g_textures[16].Sample(g_sampler, input.uv * input.tilingFactor); break;
        case 17: result = g_textures[17].Sample(g_sampler, input.uv * input.tilingFactor); break;
        case 18: result = g_textures[18].Sample(g_sampler, input.uv * input.tilingFactor); break;
        case 19: result = g_textures[19].Sample(g_sampler, input.uv * input.tilingFactor); break;
        case 20: result = g_textures[20].Sample(g_sampler, input.uv * input.tilingFactor); break;
        case 21: result = g_textures[21].Sample(g_sampler, input.uv * input.tilingFactor); break;
        case 22: result = g_textures[22].Sample(g_sampler, input.uv * input.tilingFactor); break;
        case 23: result = g_textures[23].Sample(g_sampler, input.uv * input.tilingFactor); break;
        case 24: result = g_textures[24].Sample(g_sampler, input.uv * input.tilingFactor); break;
        case 25: result = g_textures[25].Sample(g_sampler, input.uv * input.tilingFactor); break;
        case 26: result = g_textures[26].Sample(g_sampler, input.uv * input.tilingFactor); break;
        case 27: result = g_textures[27].Sample(g_sampler, input.uv * input.tilingFactor); break;
        case 28: result = g_textures[28].Sample(g_sampler, input.uv * input.tilingFactor); break;
        case 29: result = g_textures[29].Sample(g_sampler, input.uv * input.tilingFactor); break;
        case 30: result = g_textures[30].Sample(g_sampler, input.uv * input.tilingFactor); break;
        case 31: result = g_textures[31].Sample(g_sampler, input.uv * input.tilingFactor); break;
    }

    output.color    = result * input.color;
    output.objectID = input.id;

    return output;
}
