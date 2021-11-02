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
        float           Thickness;
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
        static const uint32_t MaxQuads = 20000;
        static const uint32_t MaxVertices = MaxQuads * 4;
        static const uint32_t MaxIndices = MaxQuads * 6;
        static const uint32_t MaxTextureSlots = 32;

        std::shared_ptr<Texture2D> WhiteTexture;
        std::shared_ptr<Shader> TextureShader;
        uint32_t QuadIndexCount = 0;
        std::unique_ptr<QuadVertex> QuadVertexBufferBase;
        QuadVertex *QuadVertexBufferPtr = nullptr;

        std::array<std::shared_ptr<Texture2D>, MaxTextureSlots> TextureSlots;
        uint32_t TextureSlotIndex = 1; // 0 = white texture

        Vector4 QuadVertexPositions[4];

        Statistics Stats;
    };

public:
    static void INIT();
    static void Shutdown();

    static void BeginScene(const Camera& camera);
    static void BeginScene(const Camera& camera, const Matrix4& transform);

    static void BeginScene(const OrthographicCamera camera);
    static void EndScene();
    static void Flush();

    static void SetColor(const Vector4 &color, const float brightness, const Vector3 HSV = Vector3(0.0f));

    static void DrawQuad(const Vector2& position, const Vector2& size, const Vector4& color);
    static void DrawQuad(const Vector3& position, const Vector2& size, const Vector4& color);
    static void DrawQuad(const Vector2& position, const Vector2& size, const std::shared_ptr<Texture2D>& texture, float tilingFactor = 1.0f, const Vector4& tintColor = Vector4(1.0f));
    static void DrawQuad(const Vector3& position, const Vector2& size, const std::shared_ptr<Texture2D>& texture, float tilingFactor = 1.0f, const Vector4& tintColor = Vector4(1.0f));

    static void DrawQuad(const Matrix4& transform, const Vector4& color, int entityID = -1);
    static void DrawQuad(const Matrix4& transform, const std::shared_ptr<Texture2D>& texture, float tilingFactor = 1.0f, const Vector4& tintColor = Vector4(1.0f), int entityID = -1);

    static void DrawRotatedQuad(const Vector2& position, const Vector2& size, float rotation, const Vector4& color);
    static void DrawRotatedQuad(const Vector3& position, const Vector2& size, float rotation, const Vector4& color);
    static void DrawRotatedQuad(const Vector2& position, const Vector2& size, float rotation, const std::shared_ptr<Texture2D>& texture, float tilingFactor = 1.0f, const Vector4& tintColor = Vector4(1.0f));
    static void DrawRotatedQuad(const Vector3& position, const Vector2& size, float rotation, const std::shared_ptr<Texture2D>& texture, float tilingFactor = 1.0f, const Vector4& tintColor = Vector4(1.0f));

    static void DrawSprite(const Matrix4& transform, SpriteRendererComponent& src, int entityID);

    static void ResetStats();

public:
    static Data data;

    static Statistics Stats();

    static std::shared_ptr<Pipeline> pipeline;

private:
    static void StartBatch();
    static void NextBatch();
};
}

