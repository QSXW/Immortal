#include "impch.h"
#include "Render2D.h"

#include "Render.h"
#include "FileSystem/FileSystem.h"
#include <array>

namespace Immortal
{

Render2D::Data Render2D::data;

std::shared_ptr<GraphicsPipeline> Render2D::pipeline{ nullptr };

std::shared_ptr<Buffer> Render2D::uniform{ nullptr };

void Render2D::Setup()
{
    data.textureDescriptors.reset(Render::CreateDescriptor<Texture>(Data::MaxTextureSlots));

    data.QuadVertexBufferBase.reset(new QuadVertex[data.MaxVertices]);
    pipeline.reset(Render::Create<Pipeline::Graphics>(Render::Get<Shader, ShaderName::Render2D>()));
    uniform.reset(Render::Create<Buffer>(sizeof(Matrix4), 0));

    pipeline->Set({
        { Format::VECTOR3, "POSITION"      },
        { Format::VECTOR4, "COLOR"         },
        { Format::VECTOR2, "TEXCOORD"      },
        { Format::FLOAT,   "INDEX"         },
        { Format::FLOAT,   "TILING_FACTOR" },
        { Format::R32,     "OBJECT_ID"     }
    });

    pipeline->Set(std::shared_ptr<Buffer>{ Render::Create<Buffer>(data.MaxVertices, Buffer::Type::Vertex) });

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
        pipeline->Set(std::shared_ptr<Buffer>{ Render::CreateBuffer<uint32_t>(data.MaxIndices, quadIndices.get(), Buffer::Type::Index) });
    }
    pipeline->Create(Render::Preset()->Target);

    data.WhiteTexture = Render::Preset()->WhiteTexture;

    for (uint32_t i = 0; i < data.MaxTextureSlots; i++)
    {
        data.WhiteTexture->As(data.textureDescriptors.get(), i);
        data.ActiveTextures[i] = data.WhiteTexture;
    }

    pipeline->Bind("UBO", uniform.get());
    pipeline->Bind(data.textureDescriptors.get(), 1);

    data.QuadVertexPositions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
    data.QuadVertexPositions[1] = {  0.5f, -0.5f, 0.0f, 1.0f };
    data.QuadVertexPositions[2] = {  0.5f,  0.5f, 0.0f, 1.0f };
    data.QuadVertexPositions[3] = { -0.5f,  0.5f, 0.0f, 1.0f };

    data.QuadVertexBufferPtr = data.QuadVertexBufferBase.get();
}

void Render2D::Shutdown()
{

}

void Render2D::Flush()
{
    if (!data.QuadIndexCount)
    {
        return;
    }

    uint32_t dataSize = data.QuadVertexBufferPtr - data.QuadVertexBufferBase.get();
    pipeline->Update(dataSize, data.QuadVertexBufferBase.get());

    if (isTextureChanged)
    {
        pipeline->Bind(data.textureDescriptors.get(), 1);
        isTextureChanged = false;
    }

    pipeline->ElementCount = data.QuadIndexCount;
    Render::Draw(pipeline);

    data.QuadIndexCount = 0;
    data.QuadVertexBufferPtr = data.QuadVertexBufferBase.get();
    data.Stats.DrawCalls++;
}

void Render2D::SetColor(const Vector4 &color, const float brightness, const Vector3 HSV)
{

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
        data.QuadVertexBufferPtr->Position     = transform * data.QuadVertexPositions[i];
        data.QuadVertexBufferPtr->Color        = color;
        data.QuadVertexBufferPtr->TexCoord     = textureCoords[i];
        data.QuadVertexBufferPtr->TexIndex     = textureIndex;
        data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
        data.QuadVertexBufferPtr->EntityID     = entityID;
        data.QuadVertexBufferPtr++;
    }
    data.QuadIndexCount += 6; /* One quad needs 4 vertices and the index count is 6 */
    data.Stats.QuadCount++;
}

void Render2D::DrawQuad(const Matrix4 &transform, const std::shared_ptr<Texture> &texture, float tilingFactor, const Vector4 &tintColor, int entityID)
{
    constexpr size_t quadVertexCount = 4;
    constexpr Vector2 textureCoords[] = {
        { 0.0f, 0.0f },
        { 1.0f, 0.0f },
        { 1.0f, 1.0f },
        { 0.0f, 1.0f }
    };
        
    if (data.QuadIndexCount >= Data::MaxIndices || data.TextureSlotIndex >= Data::MaxTextureSlots)
    {
        NextBatch();
    }

    float textureIndex = 0.0f;
    size_t i = 0;
    for (i = 0; i < SL_ARRAY_LENGTH(data.ActiveTextures); i++)
    {
        if (*data.ActiveTextures[i] == *texture)
        {
            break;
        }
    }
    if (i == SL_ARRAY_LENGTH(data.ActiveTextures))
    {
        data.ActiveTextures[data.TextureSlotIndex] = texture;
        texture->As(data.textureDescriptors.get(), data.TextureSlotIndex);
        isTextureChanged = true;
        data.TextureSlotIndex++;
        textureIndex = static_cast<float>(data.TextureSlotIndex);
    }
    else
    {
        textureIndex = static_cast<float>(i);
    }

    for (size_t i = 0; i < quadVertexCount; i++)
    {
        data.QuadVertexBufferPtr->Position     = transform * data.QuadVertexPositions[i];
        data.QuadVertexBufferPtr->Color        = tintColor;
        data.QuadVertexBufferPtr->TexCoord     = textureCoords[i];
        data.QuadVertexBufferPtr->TexIndex     = textureIndex;
        data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
        data.QuadVertexBufferPtr->EntityID     = entityID;
        data.QuadVertexBufferPtr++;
    }
    data.QuadIndexCount += 6; /* One quad needs 4 vertices and the index count is 6 */
    data.Stats.QuadCount++;
}

Render2D::Statistics Render2D::Stats()
{
    return data.Stats;
}

}
