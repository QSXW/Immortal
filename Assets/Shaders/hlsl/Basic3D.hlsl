struct VSInput
{
    float4 position  : POSITION;
    float3 normal    : NORMAL;
    float3 tangent   : TANGENT;
    float3 biTangent : BITANGENT;
    float2 uv        : TEXCOORD;
};

struct PSInput
{
    float4 position  : SV_POSITION;
    float3 worldPos  : WORLD_POSITION;
    float3 normal    : NORMAL;
    float3 tangent   : TANGENT;
    float3 biTangent : BITANGENT;
    float2 uv        : TEXCOORD;
    int    id        : OBJECT_ID;
};

struct Model
{
    float4x4 transform;
    float3   color;
    float    roughness;
    float    metallic;
    int      objectID;
};

ConstantBuffer<Model> model : register(b8);

cbuffer ubo : register(b0)
{
	float4x4 viewProjection;
    float4x4 skyProjection;
    float4x4 sceneRotation;
};

struct Light {
    float3 position;
    float3 radiance;
};

cbuffer shading : register(b1)
{
    Light  lights[4];
    float3 camPos;
    float  exposure;
    float  gamma;
};

Texture2D AlbodoMap    : register(t0);
Texture2D NormalMap    : register(t1);
Texture2D MetallicMap  : register(t2);
Texture2D RoughnessMap : register(t3);
Texture2D AOMap        : register(t4);
SamplerState g_sampler : register(s0);

PSInput VSMain(VSInput input)
{
    PSInput result;

    input.position.y = -input.position.y;
    float4 worldPos  = mul(model.transform, input.position);

    result.worldPos     = (float3)worldPos;
    result.position     = mul(viewProjection, worldPos);
    result.normal       = mul((float3x3)model.transform, input.normal);
    result.tangent      = input.tangent;
    result.biTangent    = input.biTangent;
    result.uv           = input.uv;
    result.id           = model.objectID;

    return result;
}

struct PSOutput
{
    float4 color : SV_TARGET;
    int objectID : COLOR;
};

PSOutput PSMain(PSInput input) : SV_TARGET
{
    PSOutput output;

    float3 objectColor = AlbodoMap.Sample(g_sampler, input.uv).rgb * model.color;
    float3 N = normalize(input.normal);

    output.color    = float4(objectColor.xyz, 1.0f);
    output.objectID = model.objectID;

    return output;
}
