#include "impch.h"
#include "Renderer2D.h"

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
    Vector::Vector3 WorldPosition;
    float           Thickness;
    Vector::Vector2 LocalPosition;
    Vector::Vector4 Color;
};

struct Renderer2DData
{
    static const uint32_t MaxQuads = 20000;
    static const uint32_t MaxVertices = MaxQuads * 4;
    static const uint32_t MaxIndices = MaxQuads * 6;
    static const uint32_t MaxTextureSlots = 32;

    Ref<VertexArray> QuadVertexArray;
    Ref<VertexBuffer> QuadVertexBuffer;
    Ref<Shader> TextureShader;
    Ref<Texture2D> WhiteTexture;

    uint32_t QuadIndexCount = 0;
    QuadVertex* QuadVertexBufferBase = nullptr;
    QuadVertex* QuadVertexBufferPtr = nullptr;

    std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;
    uint32_t TextureSlotIndex = 1; // 0 = white texture

    Vector::Vector4 QuadVertexPositions[4];

    Renderer2D::Statistics Stats;
};

static Renderer2DData sData;

void Renderer2D::INIT()
{
    sData.QuadVertexArray = Immortal::VertexArray::Create();

    sData.QuadVertexBuffer = VertexBuffer::Create(sData.MaxVertices * sizeof(QuadVertex));
    sData.QuadVertexBuffer->SetLayout({
        { Shader::DataType::Float3, "a_Position"     },
        { Shader::DataType::Float4, "a_Color"        },
        { Shader::DataType::Float2, "a_TexCoord"     },
        { Shader::DataType::Float,  "a_TexIndex"     },
        { Shader::DataType::Float,  "a_TilingFactor" },
        { Shader::DataType::Int,    "a_EntityID"     }
        });
    sData.QuadVertexArray->AddVertexBuffer(sData.QuadVertexBuffer);
        
    sData.QuadVertexBufferBase = new QuadVertex[sData.MaxVertices];

    uint32_t *quadIndices = new uint32_t[sData.MaxIndices];
    uint32_t offset = 0;

    for (uint32_t i = 0; i < sData.MaxIndices; i += 6)
    {
        quadIndices[i + 0] = offset + 0;
        quadIndices[i + 1] = offset + 1;
        quadIndices[i + 2] = offset + 2;

        quadIndices[i + 3] = offset + 2;
        quadIndices[i + 4] = offset + 3;
        quadIndices[i + 5] = offset + 0;

        offset += 4;
    }

    Ref<IndexBuffer> quadIndexBuffer = IndexBuffer::Create(quadIndices, sData.MaxIndices);
    sData.QuadVertexArray->SetIndexBuffer(quadIndexBuffer);
    delete[] quadIndices;

    sData.WhiteTexture = Texture2D::Create(1, 1);
    uint32_t whiteTextureData = 0xffffffff;
    sData.WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

    // texture slots with order
    int32_t samplers[sData.MaxTextureSlots];
    for (uint32_t i = 0; i < sData.MaxTextureSlots; i++)
    {
        samplers[i] = i;
    }

    sData.TextureShader = Render::Get<Shader, ShaderName::Texture>();
    sData.TextureShader->Map();
    sData.TextureShader->SetUniform("u_Textures", samplers, sData.MaxTextureSlots);

    // Set first texture slot = 0;
    sData.TextureSlots[0] = sData.WhiteTexture;

    sData.QuadVertexPositions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
    sData.QuadVertexPositions[1] = {  0.5f, -0.5f, 0.0f, 1.0f };
    sData.QuadVertexPositions[2] = {  0.5f,  0.5f, 0.0f, 1.0f };
    sData.QuadVertexPositions[3] = { -0.5f,  0.5f, 0.0f, 1.0f };
}

void Renderer2D::Shutdown()
{
    delete[] sData.QuadVertexBufferBase;
}

void Renderer2D::BeginScene(const Camera & camera)
{
    sData.TextureShader->Map();
    sData.TextureShader->SetUniform("u_ViewProjection", camera.ViewProjection());

    StartBatch();
}

void Renderer2D::BeginScene(const Camera & camera, const Vector::mat4 & transform)
{
    sData.TextureShader->Map();
    sData.TextureShader->SetUniform("u_ViewProjection", camera.Projection() * Vector::Inverse(transform));

    StartBatch();
}

void Renderer2D::BeginScene(const OrthographicCamera camera)
{
    sData.TextureShader->Map();
    sData.TextureShader->SetUniform("u_ViewProjection", camera.ViewPorjectionMatrix());

    StartBatch();
}

void Renderer2D::EndScene()
{
    Flush();
    sData.TextureShader->Unmap();
}

void Renderer2D::Flush()
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

void Renderer2D::SetColor(const Vector::Vector4 &color, const float brightness, const Vector::Vector3 HSV)
{
    sData.TextureShader->SetUniform("u_RGBA", color);
    sData.TextureShader->SetUniform("u_Luminance", brightness);
    sData.TextureShader->SetUniform("u_HSV", HSV);
}

void Renderer2D::DrawQuad(const Vector::Vector2 & position, const Vector::Vector2 & size, const Vector::Vector4 & color)
{
    DrawQuad({ position.x, position.y, 0.0f }, size, color);
}

void Renderer2D::DrawQuad(const Vector::Vector3 & position, const Vector::Vector2 & size, const Vector::Vector4 & color)
{
    Vector::mat4 transform = Vector::Translate(position) *
        Vector::Scale({ size.x, size.y, 1.0f });
    DrawQuad(transform, color);
}

void Renderer2D::DrawQuad(const Vector::Vector2 & position, const Vector::Vector2 & size, const Ref<Texture2D>& texture, float tilingFactor, const Vector::Vector4 & tintColor)
{
    DrawQuad({ position.x, position.y, 0.0f }, size, texture, tilingFactor, tintColor);
}

void Renderer2D::DrawQuad(const Vector::Vector3 & position, const Vector::Vector2 & size, const Ref<Texture2D>& texture, float tilingFactor, const Vector::Vector4 & tintColor)
{
    Vector::mat4 transform = Vector::Translate(position) * Vector::Scale({ size.x, size.y, 1.0f });
    DrawQuad(transform, texture, tilingFactor, tintColor);
}

void Renderer2D::DrawQuad(const Vector::mat4 & transform, const Vector::Vector4 & color, int entityID)
{
    constexpr size_t quadVertexCount = 4;
    constexpr float textureIndex = 0.0f;
    constexpr Vector::Vector2 textureCoords[] = {
        { 0.0f, 0.0f },
        { 1.0f, 0.0f },
        { 1.0f, 1.0f },
        { 0.0f, 1.0f }
    };
    constexpr float tilingFactor = 1.0f;

    if (sData.QuadIndexCount >= Renderer2DData::MaxIndices)
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

void Renderer2D::DrawQuad(const Vector::mat4 & transform, const Ref<Texture2D>& texture, float tilingFactor, const Vector::Vector4 & tintColor, int entityID)
{
    constexpr size_t quadVertexCount = 4;
    constexpr Vector::Vector2 textureCoords[] = {
        { 0.0f, 0.0f },
        { 1.0f, 0.0f },
        { 1.0f, 1.0f },
        { 0.0f, 1.0f }
    };
        
    if (sData.QuadIndexCount >= Renderer2DData::MaxIndices)
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
        if (sData.TextureSlotIndex >= Renderer2DData::MaxTextureSlots)
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

void Renderer2D::DrawRotatedQuad(const Vector::Vector2 & position, const Vector::Vector2 & size, float rotation, const Vector::Vector4 & color)
{
    DrawRotatedQuad({ position.x, position.y, 0.0f }, size, rotation, color);
}

void Renderer2D::DrawRotatedQuad(const Vector::Vector3 & position, const Vector::Vector2 & size, float rotation, const Vector::Vector4 & color)
{
    Vector::mat4 transform = Vector::Translate(position)
        * Vector::Rotate(rotation, { 0.0f, 0.0f, 1.0f })
        * Vector::Scale({ size.x, size.y, 1.0f });
    DrawQuad(transform, color);
}

void Renderer2D::DrawRotatedQuad(const Vector::Vector2 & position, const Vector::Vector2 & size, float rotation, const Ref<Texture2D>& texture, float tilingFactor, const Vector::Vector4 & tintColor)
{
    DrawRotatedQuad({ position.x, position.y, 0.0f }, size, rotation, texture, tilingFactor, tintColor);
}

void Renderer2D::DrawRotatedQuad(const Vector::Vector3 & position, const Vector::Vector2 & size, float rotation, const Ref<Texture2D>& texture, float tilingFactor, const Vector::Vector4 & tintColor)
{
    Vector::mat4 transform = Vector::Translate(position) * Vector::Rotate(rotation, { 0.0f, 0.0f, 1.0f }) * Vector::Scale({ size.x, size.y, 1.0f });
    DrawQuad(transform, texture, tilingFactor, tintColor);
}

void Renderer2D::DrawSprite(const Vector::Matrix4 & transform, SpriteRendererComponent & src, int entityID)
{
    DrawQuad(transform, src.Texture, src.TilingFactor, src.Color, entityID);
}

void Renderer2D::ResetStats()
{
    memset(&sData.Stats, 0, sizeof(Statistics));
}

Renderer2D::Statistics Renderer2D::Stats()
{
    return sData.Stats;
}

void Renderer2D::StartBatch()
{
    sData.QuadIndexCount = 0;
    sData.QuadVertexBufferPtr = sData.QuadVertexBufferBase; // reset to start
    sData.TextureSlotIndex = 1;
}

void Renderer2D::NextBatch()
{
    Flush();
    StartBatch();
}

}
