#include "impch.h"
#include "Render2D.h"

#include "Render.h"
#include <array>

namespace Immortal
{

Render2D::Data Render2D::data;

std::shared_ptr<Pipeline> Render2D::pipeline{ nullptr };

void Render2D::INIT()
{
    data.QuadVertexBufferBase.reset(new QuadVertex[data.MaxVertices]);
    pipeline = Render::CreatePipeline(Render::Get<Shader, ShaderName::Render2D>());
    pipeline->Set({
        { Format::VECTOR3, "Position"     },
        { Format::VECTOR4, "Color"        },
        { Format::VECTOR2, "TexCoord"     },
        { Format::FLOAT,   "TexIndex"     },
        { Format::FLOAT,   "TilingFactor" },
        { Format::INT,     "EntityID"     }
        });
    pipeline->Set(Render::Create<Buffer>(data.MaxVertices, Buffer::Type::Vertex));

    {
        std::unique_ptr<uint32_t> quadIndices;
        quadIndices.reset(new uint32_t[data.MaxIndices]);

        auto ptr = quadIndices.get();
        for (uint32_t i = 0, offset = 0; i < data.MaxIndices; i += 6)
        {
            ptr[i + 0] = offset + 0;
            ptr[i + 1] = offset + 1;
            ptr[i + 2] = offset + 2;

            ptr[i + 3] = offset + 2;
            ptr[i + 4] = offset + 3;
            ptr[i + 5] = offset + 0;

            offset += 4;
        }
        pipeline->Set(Render::CreateBuffer<uint32_t>(data.MaxIndices, quadIndices.get(), Buffer::Type::Index));
    }

    //data.WhiteTexture = Texture2D::Create(1, 1);
    //uint32_t whiteTextureData = 0xffffffff;
    //data.WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

    //// texture slots with order
    //int32_t samplers[data.MaxTextureSlots];
    //for (uint32_t i = 0; i < data.MaxTextureSlots; i++)
    //{
    //    samplers[i] = i;
    //}

    //data.TextureShader = Render::Get<Shader, ShaderName::Texture>();
    //data.TextureShader->Map();
    //data.TextureShader->Set("u_Textures", samplers, data.MaxTextureSlots);

    //// Set first texture slot = 0;
    //data.TextureSlots[0] = data.WhiteTexture;

    //data.QuadVertexPositions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
    //data.QuadVertexPositions[1] = {  0.5f, -0.5f, 0.0f, 1.0f };
    //data.QuadVertexPositions[2] = {  0.5f,  0.5f, 0.0f, 1.0f };
    //data.QuadVertexPositions[3] = { -0.5f,  0.5f, 0.0f, 1.0f };
}

void Render2D::Shutdown()
{

}

void Render2D::BeginScene(const Camera &camera)
{
    data.TextureShader->Map();
    data.TextureShader->Set("u_ViewProjection", camera.ViewProjection());

    StartBatch();
}

void Render2D::BeginScene(const Camera &camera, const Matrix4 &transform)
{
    data.TextureShader->Map();
    data.TextureShader->Set("u_ViewProjection", camera.Projection() * Vector::Inverse(transform));

    StartBatch();
}

void Render2D::BeginScene(const OrthographicCamera camera)
{
    data.TextureShader->Map();
    data.TextureShader->Set("u_ViewProjection", camera.ViewProjection());

    StartBatch();
}

void Render2D::EndScene()
{
    Flush();
    data.TextureShader->Unmap();
}

void Render2D::Flush()
{
    /* Nothing to draw */
    if (data.QuadIndexCount == 0)
    {
        return;
    }

    /* Calculate Vertex data size and submit */
    uint32_t dataSize = data.QuadVertexBufferPtr - data.QuadVertexBufferBase.get();
    pipeline->Update(dataSize, data.QuadVertexBufferBase.get());

    /* Map Texture */
    for (uint32_t i = 0; i < data.TextureSlotIndex; i++)
    {
        data.TextureSlots[i]->Map(i);
    }

    Render::Draw(pipeline);
    data.Stats.DrawCalls++;
}

void Render2D::SetColor(const Vector4 &color, const float brightness, const Vector3 HSV)
{
    data.TextureShader->Set("u_RGBA", color);
    data.TextureShader->Set("u_Luminance", brightness);
    data.TextureShader->Set("u_HSV", HSV);
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

    if (data.QuadIndexCount >= Data::MaxIndices)
    {
        NextBatch();
    }

    for (size_t i = 0; i < quadVertexCount; i++)
    {
        data.QuadVertexBufferPtr->Position = transform * data.QuadVertexPositions[i];
        data.QuadVertexBufferPtr->Color = color;
        data.QuadVertexBufferPtr->TexCoord = textureCoords[i];
        data.QuadVertexBufferPtr->TexIndex = textureIndex;
        data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
        data.QuadVertexBufferPtr->EntityID = entityID;
        data.QuadVertexBufferPtr++;
    }
    data.QuadIndexCount += 6; /* One quad needs 4 vertices and the index count is 6 */
    data.Stats.QuadCount++;
}

void Render2D::DrawQuad(const Matrix4 &transform, const std::shared_ptr<Texture>&texture, float tilingFactor, const Vector4 &tintColor, int entityID)
{
    constexpr size_t quadVertexCount = 4;
    constexpr Vector2 textureCoords[] = {
        { 0.0f, 0.0f },
        { 1.0f, 0.0f },
        { 1.0f, 1.0f },
        { 0.0f, 1.0f }
    };
        
    if (data.QuadIndexCount >= Data::MaxIndices)
    {
        NextBatch();
    }

    float textureIndex = 0.0f;
    for (uint32_t i = 1; i < data.TextureSlotIndex; i++)
    {
        if (*data.TextureSlots[i] == *texture)
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
        if (data.TextureSlotIndex >= Data::MaxTextureSlots)
        {
            NextBatch();
        }
        textureIndex = static_cast<float>(data.TextureSlotIndex);
        data.TextureSlots.at(data.TextureSlotIndex) = texture;
        data.TextureSlotIndex++;
    }

    for (size_t i = 0; i < quadVertexCount; i++)
    {
        data.QuadVertexBufferPtr->Position = transform * data.QuadVertexPositions[i];
        data.QuadVertexBufferPtr->Color = tintColor;
        data.QuadVertexBufferPtr->TexCoord = textureCoords[i];
        data.QuadVertexBufferPtr->TexIndex = textureIndex;
        data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
        data.QuadVertexBufferPtr->EntityID = entityID;
        data.QuadVertexBufferPtr++;
    }
    data.QuadIndexCount += 6; /* One quad needs 4 vertices and the index count is 6 */
    data.Stats.QuadCount++;
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
    memset(&data.Stats, 0, sizeof(Statistics));
}

Render2D::Statistics Render2D::Stats()
{
    return data.Stats;
}

void Render2D::StartBatch()
{
    data.QuadIndexCount = 0;
    data.QuadVertexBufferPtr = data.QuadVertexBufferBase.get(); // reset to start
    data.TextureSlotIndex = 1;
}

void Render2D::NextBatch()
{
    Flush();
    StartBatch();
}

}
