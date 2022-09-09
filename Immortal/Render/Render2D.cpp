#include "Render2D.h"

#include "Render.h"
#include "FileSystem/FileSystem.h"
#include <array>

namespace Immortal
{

Render2D::Data Render2D::data;

Ref<GraphicsPipeline> Render2D::pipeline{ nullptr };

Ref<Buffer> Render2D::uniform;

void Render2D::Setup()
{
	data.textureDescriptorBuffer = Render::CreateDescriptor<Texture>(Data::MaxTextureSlots);

    data.RectVertexBuffer.resize(data.MaxVertices);
    pipeline = Render::Create<Pipeline::Graphics>(Render::GetShader("Render2D"));
    uniform = Render::Create<Buffer>(sizeof(Matrix4), 0);

    pipeline->Set({
        { Format::VECTOR3, "POSITION"      },
        { Format::VECTOR4, "COLOR"         },
        { Format::VECTOR2, "TEXCOORD"      },
        { Format::FLOAT,   "INDEX"         },
        { Format::FLOAT,   "TILING_FACTOR" },
        { Format::R32,     "OBJECT_ID"     }
    });

    pipeline->Set(Render::Create<Buffer>(data.MaxVertices * sizeof(RectVertex), Buffer::Type::Vertex));

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
        pipeline->Set(Render::Create<Buffer>(data.MaxIndices * sizeof(uint32_t), rectIndices.get(), Buffer::Type::Index));
    }
    pipeline->Enable(Pipeline::State::Blend);
    pipeline->Create(Render::Preset()->Target);

    data.WhiteTexture = Render::Preset()->Textures.White;

    for (uint32_t i = 0; i < data.MaxTextureSlots; i++)
    {
		data.WhiteTexture->As(data.textureDescriptorBuffer, i);
        data.ActiveTextures[i] = data.WhiteTexture;
    }
    isTextureChanged = true;

    data.RectVertexPositions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
    data.RectVertexPositions[1] = {  0.5f, -0.5f, 0.0f, 1.0f };
    data.RectVertexPositions[2] = {  0.5f,  0.5f, 0.0f, 1.0f };
    data.RectVertexPositions[3] = { -0.5f,  0.5f, 0.0f, 1.0f };

    data.pRectVertex = data.RectVertexBuffer.data();
}

void Render2D::Release()
{
	data.textureDescriptorBuffer.Reset();
    uniform.Reset();
    data.WhiteTexture.Reset();
    data.RectVertexBuffer.clear();
    pipeline.Reset();

    for (auto &t : data.ActiveTextures)
    {
        t.Reset();
    }
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
        pipeline->Bind(uniform,                       0);
		pipeline->Bind(data.textureDescriptorBuffer,  1);
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
    static Vector2 textureCoords[] = {
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
        data.pRectVertex->Position     = Vector4{ transform * data.RectVertexPositions[i] };
        data.pRectVertex->Color        = color;
        data.pRectVertex->TexCoord     = textureCoords[i];
        data.pRectVertex->TexIndex     = textureIndex;
        data.pRectVertex->TilingFactor = tilingFactor;
        data.pRectVertex->Object       = object;
    }
    data.RectIndexCount += 6;
    data.Stats.RectCount++;
}

void Render2D::DrawRect(const Matrix4 &transform, const Ref<Texture> &texture, float tilingFactor, const Vector4 &tintColor, int object)
{
    constexpr size_t RectVertexCount = 4;
    static Vector2 textureCoords[] = {
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
		texture->As(data.textureDescriptorBuffer, data.TextureSlotIndex);
        textureIndex = data.TextureSlotIndex++;
        isTextureChanged = true;
    }

    for (size_t i = 0; i < RectVertexCount; i++, data.pRectVertex++)
    {
        data.pRectVertex->Position     = Vector4{ transform * data.RectVertexPositions[i] };
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
