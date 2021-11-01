#include "impch.h"
#include "Render2D.h"

#include "Render.h"
#include <array>

namespace Immortal
{

struct QuadVertex
{
    Vector3 Position;
    Vector4 Color;
    Vector2 TexCoord;
    float TexIndex;
    float TilingFactor;
    int EntityID;
};

struct LineVertex
{
    Vector3 Position;
    Vector4 Color;
};

struct CircleVertex
{
    Vector3 WorldPosition;
    float           Thickness;
    Vector2 LocalPosition;
    Vector4 Color;
};

struct Render2DData
{
    static const uint32_t MaxQuads = 20000;
    static const uint32_t MaxVertices = MaxQuads * 4;
    static const uint32_t MaxIndices = MaxQuads * 6;
    static const uint32_t MaxTextureSlots = 32;

    std::shared_ptr<VertexArray> QuadVertexArray;
    std::shared_ptr<VertexBuffer> QuadVertexBuffer;
    std::shared_ptr<Shader> TextureShader;
    std::shared_ptr<Texture2D> WhiteTexture;

    uint32_t QuadIndexCount = 0;
    QuadVertex* QuadVertexBufferBase = nullptr;
    QuadVertex* QuadVertexBufferPtr = nullptr;

    std::array<std::shared_ptr<Texture2D>, MaxTextureSlots> TextureSlots;
    uint32_t TextureSlotIndex = 1; // 0 = white texture

    Vector4 QuadVertexPositions[4];

    Render2D::Statistics Stats;
};

static Render2DData sData;

std::shared_ptr<Pipeline> Render2D::pipeline{ nullptr };

void Render2D::INIT()
{
    pipeline = Render::CreatePipeline(Render::Get<Shader, ShaderName::Render2D>());
    pipeline->Set({
        { Format::VECTOR3, "Position"     },
        { Format::VECTOR4, "Color"        },
        { Format::VECTOR2, "TexCoord"     },
        { Format::FLOAT,   "TexIndex"     },
        { Format::FLOAT,   "TilingFactor" },
        { Format::INT,     "EntityID"     }
        });
    pipeline->Set(Render::CreateBuffer<QuadVertex>(sData.MaxVertices, Buffer::Type::Vertex));

    {
        std::unique_ptr<uint32_t> quadIndices;
        quadIndices.reset(new uint32_t[sData.MaxIndices]);

        auto ptr = quadIndices.get();
        for (uint32_t i = 0, offset = 0; i < sData.MaxIndices; i += 6)
        {
            ptr[i + 0] = offset + 0;
            ptr[i + 1] = offset + 1;
            ptr[i + 2] = offset + 2;

            ptr[i + 3] = offset + 2;
            ptr[i + 4] = offset + 3;
            ptr[i + 5] = offset + 0;

            offset += 4;
        }
        pipeline->Set(Render::CreateBuffer<uint32_t>(sData.MaxIndices, quadIndices.get(), Buffer::Type::Index));
    }

    //sData.WhiteTexture = Texture2D::Create(1, 1);
    //uint32_t whiteTextureData = 0xffffffff;
    //sData.WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

    //// texture slots with order
    //int32_t samplers[sData.MaxTextureSlots];
    //for (uint32_t i = 0; i < sData.MaxTextureSlots; i++)
    //{
    //    samplers[i] = i;
    //}

    //sData.TextureShader = Render::Get<Shader, ShaderName::Texture>();
    //sData.TextureShader->Map();
    //sData.TextureShader->Set("u_Textures", samplers, sData.MaxTextureSlots);

    //// Set first texture slot = 0;
    //sData.TextureSlots[0] = sData.WhiteTexture;

    //sData.QuadVertexPositions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
    //sData.QuadVertexPositions[1] = {  0.5f, -0.5f, 0.0f, 1.0f };
    //sData.QuadVertexPositions[2] = {  0.5f,  0.5f, 0.0f, 1.0f };
    //sData.QuadVertexPositions[3] = { -0.5f,  0.5f, 0.0f, 1.0f };
}

void Render2D::Shutdown()
{
    delete[] sData.QuadVertexBufferBase;
}

void Render2D::BeginScene(const Camera &camera)
{
    sData.TextureShader->Map();
    sData.TextureShader->Set("u_ViewProjection", camera.ViewProjection());

    StartBatch();
}

void Render2D::BeginScene(const Camera &camera, const Matrix4 &transform)
{
    sData.TextureShader->Map();
    sData.TextureShader->Set("u_ViewProjection", camera.Projection() * Vector::Inverse(transform));

    StartBatch();
}

void Render2D::BeginScene(const OrthographicCamera camera)
{
    sData.TextureShader->Map();
    sData.TextureShader->Set("u_ViewProjection", camera.ViewPorjectionMatrix());

    StartBatch();
}

void Render2D::EndScene()
{
    Flush();
    sData.TextureShader->Unmap();
}

void Render2D::Flush()
{
    /* Nothing to draw */
    if (sData.QuadIndexCount == 0)
    {
        return;
    }

    /* Calculate Vertex data size and submit */
    uint32_t dataSize = static_cast<uint32_t>(
        reinterpret_cast<uint8_t *>(sData.QuadVertexBufferPtr) - reinterpret_cast<uint8_t *>(sData.QuadVertexBufferBase)
    );
    sData.QuadVertexBuffer->SetData(sData.QuadVertexBufferBase, dataSize);

    /* Map Texture */
    for (uint32_t i = 0; i < sData.TextureSlotIndex; i++)
    {
        sData.TextureSlots[i]->Map(i);
    }

    Render::DrawIndexed(sData.QuadVertexArray, sData.QuadIndexCount);
    sData.Stats.DrawCalls++;
}

void Render2D::SetColor(const Vector4 &color, const float brightness, const Vector3 HSV)
{
    sData.TextureShader->Set("u_RGBA", color);
    sData.TextureShader->Set("u_Luminance", brightness);
    sData.TextureShader->Set("u_HSV", HSV);
}

void Render2D::DrawQuad(const Vector2 &position, const Vector2 &size, const Vector4 &color)
{
    DrawQuad({ position.x, position.y, 0.0f }, size, color);
}

void Render2D::DrawQuad(const Vector3 &position, const Vector2 &size, const Vector4 &color)
{
    Matrix4 transform = Vector::Translate(position) *
        Vector::Scale({ size.x, size.y, 1.0f });
    DrawQuad(transform, color);
}

void Render2D::DrawQuad(const Vector2 &position, const Vector2 &size, const std::shared_ptr<Texture2D>&texture, float tilingFactor, const Vector4 &tintColor)
{
    DrawQuad({ position.x, position.y, 0.0f }, size, texture, tilingFactor, tintColor);
}

void Render2D::DrawQuad(const Vector3 &position, const Vector2 &size, const std::shared_ptr<Texture2D>&texture, float tilingFactor, const Vector4 &tintColor)
{
    Matrix4 transform = Vector::Translate(position) * Vector::Scale({ size.x, size.y, 1.0f });
    DrawQuad(transform, texture, tilingFactor, tintColor);
}

void Render2D::DrawQuad(const Matrix4 &transform, const Vector4 &color, int entityID)
{
    constexpr size_t quadVertexCount  = 4;
    constexpr float textureIndex      = 0.0f;
    constexpr Vector2 textureCoords[] = {
        { 0.0f, 0.0f },
        { 1.0f, 0.0f },
        { 1.0f, 1.0f },
        { 0.0f, 1.0f }
    };
    constexpr float tilingFactor = 1.0f;

    if (sData.QuadIndexCount >= Render2DData::MaxIndices)
    {
        NextBatch();
    }

    for (size_t i = 0; i < quadVertexCount; i++)
    {
        sData.QuadVertexBufferPtr->Position = transform * sData.QuadVertexPositions[i];
        sData.QuadVertexBufferPtr->Color = color;
        sData.QuadVertexBufferPtr->TexCoord = textureCoords[i];
        sData.QuadVertexBufferPtr->TexIndex = textureIndex;
        sData.QuadVertexBufferPtr->TilingFactor = tilingFactor;
        sData.QuadVertexBufferPtr->EntityID = entityID;
        sData.QuadVertexBufferPtr++;
    }
    sData.QuadIndexCount += 6; /* One quad needs 4 vertices and the index count is 6 */
    sData.Stats.QuadCount++;
}

void Render2D::DrawQuad(const Matrix4 &transform, const std::shared_ptr<Texture2D>&texture, float tilingFactor, const Vector4 &tintColor, int entityID)
{
    constexpr size_t quadVertexCount = 4;
    constexpr Vector2 textureCoords[] = {
        { 0.0f, 0.0f },
        { 1.0f, 0.0f },
        { 1.0f, 1.0f },
        { 0.0f, 1.0f }
    };
        
    if (sData.QuadIndexCount >= Render2DData::MaxIndices)
    {
        NextBatch();
    }

    float textureIndex = 0.0f;
    for (uint32_t i = 1; i < sData.TextureSlotIndex; i++)
    {
        if (*sData.TextureSlots[i] == *texture)
        {
            textureIndex = static_cast<float>(i);
            break;
        }
    }

    /* The texture has been submit to the GPU when the texture object was created
        So we just need to upload the texture index with vertices to GPU;
    */
    if (static_cast<int>(textureIndex) == 0)
    {
        if (sData.TextureSlotIndex >= Render2DData::MaxTextureSlots)
        {
            NextBatch();
        }
        textureIndex = static_cast<float>(sData.TextureSlotIndex);
        sData.TextureSlots.at(sData.TextureSlotIndex) = texture;
        sData.TextureSlotIndex++;
    }

    for (size_t i = 0; i < quadVertexCount; i++)
    {
        sData.QuadVertexBufferPtr->Position = transform * sData.QuadVertexPositions[i];
        sData.QuadVertexBufferPtr->Color = tintColor;
        sData.QuadVertexBufferPtr->TexCoord = textureCoords[i];
        sData.QuadVertexBufferPtr->TexIndex = textureIndex;
        sData.QuadVertexBufferPtr->TilingFactor = tilingFactor;
        sData.QuadVertexBufferPtr->EntityID = entityID;
        sData.QuadVertexBufferPtr++;
    }
    sData.QuadIndexCount += 6; /* One quad needs 4 vertices and the index count is 6 */
    sData.Stats.QuadCount++;
}

void Render2D::DrawRotatedQuad(const Vector2 &position, const Vector2 &size, float rotation, const Vector4 &color)
{
    DrawRotatedQuad({ position.x, position.y, 0.0f }, size, rotation, color);
}

void Render2D::DrawRotatedQuad(const Vector3 &position, const Vector2 &size, float rotation, const Vector4 &color)
{
    Matrix4 transform = Vector::Translate(position)
        * Vector::Rotate(rotation, { 0.0f, 0.0f, 1.0f })
        * Vector::Scale({ size.x, size.y, 1.0f });
    DrawQuad(transform, color);
}

void Render2D::DrawRotatedQuad(const Vector2 &position, const Vector2 &size, float rotation, const std::shared_ptr<Texture2D>&texture, float tilingFactor, const Vector4 &tintColor)
{
    DrawRotatedQuad({ position.x, position.y, 0.0f }, size, rotation, texture, tilingFactor, tintColor);
}

void Render2D::DrawRotatedQuad(const Vector3 &position, const Vector2 &size, float rotation, const std::shared_ptr<Texture2D>&texture, float tilingFactor, const Vector4 &tintColor)
{
    Matrix4 transform = Vector::Translate(position) * Vector::Rotate(rotation, { 0.0f, 0.0f, 1.0f }) * Vector::Scale({ size.x, size.y, 1.0f });
    DrawQuad(transform, texture, tilingFactor, tintColor);
}

void Render2D::DrawSprite(const Matrix4 &transform, SpriteRendererComponent &src, int entityID)
{
    DrawQuad(transform, src.Texture, src.TilingFactor, src.Color, entityID);
}

void Render2D::ResetStats()
{
    memset(&sData.Stats, 0, sizeof(Statistics));
}

Render2D::Statistics Render2D::Stats()
{
    return sData.Stats;
}

void Render2D::StartBatch()
{
    sData.QuadIndexCount = 0;
    sData.QuadVertexBufferPtr = sData.QuadVertexBufferBase; // reset to start
    sData.TextureSlotIndex = 1;
}

void Render2D::NextBatch()
{
    Flush();
    StartBatch();
}

}
