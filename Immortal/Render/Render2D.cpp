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

    data.RectVertexBuffer.resize(data.MaxVertices);
    pipeline.reset(Render::Create<Pipeline::Graphics>(Render::GetShader("Render2D")));
    uniform.reset(Render::Create<Buffer>(sizeof(Matrix4), 0));

    pipeline->Set({
        { Format::VECTOR3, "POSITION"      },
        { Format::VECTOR4, "COLOR"         },
        { Format::VECTOR2, "TEXCOORD"      },
        { Format::FLOAT,   "INDEX"         },
        { Format::FLOAT,   "TILING_FACTOR" },
        { Format::R32,     "OBJECT_ID"     }
    });

    pipeline->Set(std::shared_ptr<Buffer>{ Render::Create<Buffer>(data.MaxVertices * sizeof(RectVertex), Buffer::Type::Vertex) });

    {
        std::unique_ptr<uint32_t> rectIndices;
        rectIndices.reset(new uint32_t[data.MaxIndices]);

        auto ptr = rectIndices.get();
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
        pipeline->Set(std::shared_ptr<Buffer>{ Render::Create<Buffer>(data.MaxIndices * sizeof(uint32_t), rectIndices.get(), Buffer::Type::Index) });
    }
    pipeline->Create(Render::Preset()->Target, Pipeline::Option{ Pipeline::Feature::BlendEnabled });

    data.WhiteTexture = Render::Preset()->Textures.White;

    for (uint32_t i = 0; i < data.MaxTextureSlots; i++)
    {
        data.WhiteTexture->As(data.textureDescriptors.get(), i);
        data.ActiveTextures[i] = data.WhiteTexture;
    }

    pipeline->AllocateDescriptorSet((uint64_t)&data);
    pipeline->Bind("UBO", uniform.get());
    pipeline->Bind(data.textureDescriptors.get(), 1);

    data.RectVertexPositions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
    data.RectVertexPositions[1] = {  0.5f, -0.5f, 0.0f, 1.0f };
    data.RectVertexPositions[2] = {  0.5f,  0.5f, 0.0f, 1.0f };
    data.RectVertexPositions[3] = { -0.5f,  0.5f, 0.0f, 1.0f };

    data.pRectVertex = data.RectVertexBuffer.data();
}

void Render2D::Shutdown()
{

}

void Render2D::Flush()
{
    if (!data.RectIndexCount)
    {
        return;
    }

    uint32_t dataSize = data.pRectVertex - data.RectVertexBuffer.data();
    pipeline->Update(dataSize, data.RectVertexBuffer.data());

    if (isTextureChanged)
    {
        pipeline->AllocateDescriptorSet((uint64_t)&data);
        pipeline->Bind(data.textureDescriptors.get(), 1);
        isTextureChanged = false;
    }

    pipeline->ElementCount = data.RectIndexCount;
    Render::Draw(pipeline);

    data.RectIndexCount = 0;
    data.pRectVertex = data.RectVertexBuffer.data();
    data.Stats.DrawCalls++;
}

void Render2D::SetColor(const Vector4 &color, const float brightness, const Vector3 HSV)
{

}

void Render2D::DrawRect(const Matrix4 &transform, const Vector4 &color, int object)
{
    constexpr size_t RectVertexCount  = 4;
    constexpr float textureIndex      = 0.0f;
    constexpr Vector2 textureCoords[] = {
        { 0.0f, 0.0f },
        { 1.0f, 0.0f },
        { 1.0f, 1.0f },
        { 0.0f, 1.0f }
    };
    constexpr float tilingFactor = 1.0f;

    if (data.RectIndexCount >= Data::MaxIndices)
    {
        NextBatch();
    }

    for (size_t i = 0; i < RectVertexCount; i++, data.pRectVertex++)
    {
        data.pRectVertex->Position     = transform * data.RectVertexPositions[i];
        data.pRectVertex->Color        = color;
        data.pRectVertex->TexCoord     = textureCoords[i];
        data.pRectVertex->TexIndex     = textureIndex;
        data.pRectVertex->TilingFactor = tilingFactor;
        data.pRectVertex->Object       = object;
    }
    data.RectIndexCount += 6;
    data.Stats.RectCount++;
}

void Render2D::DrawRect(const Matrix4 &transform, const std::shared_ptr<Texture> &texture, float tilingFactor, const Vector4 &tintColor, int object)
{
    constexpr size_t RectVertexCount = 4;
    constexpr Vector2 textureCoords[] = {
        { 0.0f, 0.0f },
        { 1.0f, 0.0f },
        { 1.0f, 1.0f },
        { 0.0f, 1.0f }
    };

    if (data.RectIndexCount >= Data::MaxIndices || data.TextureSlotIndex >= Data::MaxTextureSlots)
    {
        NextBatch();
    }

    uint32_t textureIndex;
    for (textureIndex = 0; textureIndex < SL_ARRAY_LENGTH(data.ActiveTextures); textureIndex++)
    {
        if (*data.ActiveTextures[textureIndex] == *texture)
        {
            break;
        }
    }
    if (textureIndex == SL_ARRAY_LENGTH(data.ActiveTextures))
    {
        data.ActiveTextures[data.TextureSlotIndex] = texture;
        texture->As(data.textureDescriptors.get(), data.TextureSlotIndex);
        textureIndex = data.TextureSlotIndex++;
        isTextureChanged = true;
    }

    for (size_t i = 0; i < RectVertexCount; i++, data.pRectVertex++)
    {
        data.pRectVertex->Position     = transform * data.RectVertexPositions[i];
        data.pRectVertex->Color        = tintColor;
        data.pRectVertex->TexCoord     = textureCoords[i];
        data.pRectVertex->TexIndex     = textureIndex;
        data.pRectVertex->TilingFactor = tilingFactor;
        data.pRectVertex->Object       = object;
    }
    data.RectIndexCount += 6;
    data.Stats.RectCount++;
}

Render2D::Statistics Render2D::Stats()
{
    return data.Stats;
}

}
