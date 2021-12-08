#pragma once

#include "ImmortalCore.h"

#include "OrthographicCamera.h"
#include "Camera.h"
#include "Texture.h"
#include "Scene/Component.h"

namespace Immortal
{

class IMMORTAL_API Render2D
{
public:
    struct QuadVertex
    {
        Vector3 Position;
        Vector4 Color;
        Vector2 TexCoord;
        float   TexIndex;
        float   TilingFactor;
        int     EntityID;
    };

    struct LineVertex
    {
        Vector3 Position;
        Vector4 Color;
    };

    struct CircleVertex
    {
        Vector3 WorldPosition;
        float   Thickness;
        Vector2 LocalPosition;
        Vector4 Color;
    };

    struct Statistics
    {
        uint32_t DrawCalls = 0;
        uint32_t QuadCount = 0;

        uint32_t TotalVertexCount() const
        { 
            return QuadCount * 4;
        }

        uint32_t TotalIndexCount() const
        {
            return QuadCount * 6;
        }
    };

    struct Data
    {
        static constexpr uint32_t MaxQuads        = 20000;
        static constexpr uint32_t MaxVertices     = MaxQuads * 4;
        static constexpr uint32_t MaxIndices      = MaxQuads * 6;
        static constexpr uint32_t MaxTextureSlots = 32;

        std::shared_ptr<Texture> WhiteTexture;
        std::shared_ptr<Shader> TextureShader;
        std::unique_ptr<Descriptor> textureDescriptors;
        
        uint32_t QuadIndexCount = 0;
        std::unique_ptr<QuadVertex> QuadVertexBufferBase;
        QuadVertex *QuadVertexBufferPtr = nullptr;

        std::array<std::shared_ptr<Texture>, MaxTextureSlots> TextureSlots;
        uint32_t TextureSlotIndex = 1; // 0 = white texture

        Vector4 QuadVertexPositions[4];

        Statistics Stats;
    };

public:
    static void Setup();

    static void Setup(const std::shared_ptr<RenderTarget> &renderTarget)
    {
        pipeline->Reconstruct(renderTarget);
    }

    static void Shutdown();

    static void Flush();

    static void Render2D::StartBatch()
    {
        data.QuadIndexCount = 0;
        data.QuadVertexBufferPtr = data.QuadVertexBufferBase.get(); // reset to start
        data.TextureSlotIndex = 1;
    }

    static void Render2D::NextBatch()
    {
        Flush();
        StartBatch();
    }

    static void Render2D::ResetStats()
    {
        CleanUpObject(&data.Stats);
    }

    static void Render2D::BeginScene(const Camera &camera)
    {
        uniform->Update(sizeof(Matrix4), &camera.ViewProjection());

        StartBatch();
    }

    static void Render2D::BeginScene(const Camera &camera, const Matrix4 &view)
    {
        auto viewProjection = camera.Projection() * Vector::Inverse(view);
        uniform->Update(sizeof(Matrix4), &viewProjection);
        StartBatch();
    }

    static void BeginScene(const OrthographicCamera &camera)
    {
        BeginScene(dcast<const Camera &>(camera));
    }

    static void Render2D::EndScene()
    {
        Flush();
    }

    static void SetColor(const Vector4 &color, const float brightness, const Vector3 HSV = Vector3(0.0f));

    static void DrawQuad(const Matrix4 &transform, const Vector4 &color, int entityID = -1);

    static void DrawQuad(const Matrix4 &transform, const std::shared_ptr<Texture> &texture, float tilingFactor = 1.0f, const Vector4 &tintColor = Vector4(1.0f), int entityID = -1);

    static void DrawQuad(const Vector2 &position, const Vector2 &size, const Vector4 &color)
    {
        DrawQuad({ position.x, position.y, 0.0f }, size, color);
    }

    static void DrawQuad(const Vector3 &position, const Vector2 &size, const Vector4 &color)
    {
        Matrix4 transform = Vector::Translate(position) *
            Vector::Scale({ size.x, size.y, 1.0f });
        DrawQuad(transform, color);
    }

    static void DrawQuad(const Vector2 &position, const Vector2 &size, const std::shared_ptr<Texture> &texture, float tilingFactor = 1.0f, const Vector4 &tintColor = Vector4(1.0f))
    {
        DrawQuad({ position.x, position.y, 0.0f }, size, texture, tilingFactor, tintColor);
    }

    static void DrawQuad(const Vector3 &position, const Vector2 &size, const std::shared_ptr<Texture> &texture, float tilingFactor = 1.0f, const Vector4 &tintColor = Vector4(1.0f))
    {
        Matrix4 transform = Vector::Translate(position) * Vector::Scale({ size.x, size.y, 1.0f });
        DrawQuad(transform, texture, tilingFactor, tintColor);
    }

    static void DrawRotatedQuad(const Vector2 &position, const Vector2 &size, float rotation, const Vector4 &color)
    {
        DrawRotatedQuad({ position.x, position.y, 0.0f }, size, rotation, color);
    }

    static void DrawRotatedQuad(const Vector3 &position, const Vector2 &size, float rotation, const Vector4 &color)
    {
        Matrix4 transform = Vector::Translate(position)
            * Vector::Rotate(rotation, { 0.0f, 0.0f, 1.0f })
            * Vector::Scale({ size.x, size.y, 1.0f });
        DrawQuad(transform, color);
    }

    static void DrawRotatedQuad(const Vector2 &position, const Vector2 &size, float rotation, const std::shared_ptr<Texture2D> &texture, float tilingFactor = 1.0f, const Vector4 &tintColor = Vector4{ 1.0f })
    {
        DrawRotatedQuad({ position.x, position.y, 0.0f }, size, rotation, texture, tilingFactor, tintColor);
    }

    static void DrawRotatedQuad(const Vector3 &position, const Vector2 &size, float rotation, const std::shared_ptr<Texture2D> &texture, float tilingFactor = 1.0f, const Vector4 &tintColor = Vector4{ 1.0f })
    {
        Matrix4 transform = Vector::Translate(position) * Vector::Rotate(rotation, { 0.0f, 0.0f, 1.0f }) * Vector::Scale({ size.x, size.y, 1.0f });
        DrawQuad(transform, texture, tilingFactor, tintColor);
    }

    static void DrawSprite(const Matrix4 &transform, SpriteRendererComponent &src, int entityID)
    {
        DrawQuad(transform, src.Texture, src.TilingFactor, src.Color, entityID);
    }

public:
    static Data data;

    static Statistics Stats();

    static std::shared_ptr<Pipeline> pipeline;

    static std::shared_ptr<Buffer> uniform;
};

}
