struct PanoToCubemap
{
    uint CubemapSize;
};

[[vk::push_constant]] PanoToCubemap pushConstant;

Texture2D<float4>        SrcTexture : register(t0);
RWTexture2DArray<float4> Cubemap    : register(u1);
SamplerState             LinearRepeatSampler : register(s2);

static const float InvPI = 0.31830988618379067153776752674503f;
static const float Inv2PI = 0.15915494309189533576888376337251f;
static const float2 InvAtan = float2(Inv2PI, InvPI);
static const float3x3 RotateUV[6] = {
    float3x3(  0,  0,  1,
               0, -1,  0,
              -1,  0,  0 ),
    float3x3(  0,  0, -1,
               0, -1,  0,
               1,  0,  0 ),
    float3x3(  1,  0,  0,
               0,  0,  1,
               0,  1,  0 ),
    float3x3(  1,  0,  0,
               0,  0, -1,
               0, -1,  0 ),
    float3x3(  1,  0,  0,
               0, -1,  0,
               0,  0,  1 ),
    float3x3( -1,  0,  0,
               0, -1,  0,
               0,  0, -1 )
};

#define CUBE_FACE_COUNT 6
#define UNIT_X float3(1.0, 0.0, 0.0)
#define UNIT_Y float3(0.0, 1.0, 0.0)
#define UNIT_Z float3(0.0, 0.0, 1.0)

static float3 CUBE_FACES_N[CUBE_FACE_COUNT] = {
    UNIT_X,
    -UNIT_X,
    UNIT_Y,
    -UNIT_Y,
    UNIT_Z,
    -UNIT_Z
};
static float3 CUBE_FACES_T[CUBE_FACE_COUNT] = {
    -UNIT_Z,
    UNIT_Z,
    UNIT_X,
    UNIT_X,
    UNIT_X,
    -UNIT_X
};
static float3 CUBE_FACES_B[CUBE_FACE_COUNT] = {
    -UNIT_Y,
    -UNIT_Y,
    UNIT_Z,
    -UNIT_Z,
    -UNIT_Y,
    -UNIT_Y
};

float2 GetUV(uint2 id, uint2 imageSize)
{
    const float2 pixelSize = 1.0f / imageSize;
    return pixelSize * id + pixelSize * 0.5;
}

float3 GetCubeDirection(uint curFaceIndex, float2 uv)
{
    const float2 xy = uv * 2.0 - 1.0;

    const float3 normal = CUBE_FACES_N[curFaceIndex];
    const float3 tangent = CUBE_FACES_T[curFaceIndex];
    const float3 binormal = CUBE_FACES_B[curFaceIndex];

    return normalize(normal + xy.x * tangent + xy.y * binormal);
}

float2 ComputePanoramaTexCoord(float3 direction)
{
    const float2 inverseAtan = float2(0.1591, 0.3183);

    return float2(atan2(direction.z, direction.x), asin(-direction.y)) * inverseAtan + 0.5;
}

[numthreads(16, 16, 1)]
void main(int3 DTid : SV_DispatchThreadID)
{
    const float2 uv = GetUV(DTid.xy, uint2( pushConstant.CubemapSize,  pushConstant.CubemapSize));

    const float3 direction = GetCubeDirection(DTid.z, uv);

    const float2 panoramaTexCoord = ComputePanoramaTexCoord(direction);

    float4 panoramaSample = SrcTexture.SampleLevel(LinearRepeatSampler, panoramaTexCoord, 0);

    Cubemap[DTid] = panoramaSample;
}
