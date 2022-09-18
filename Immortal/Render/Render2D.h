#pragma once

#include "Core.h"

#include "Camera.h"
#include "Texture.h"
#include "Scene/Component.h"
#include "Math/Vector.h"

namespace Immortal
{

class IMMORTAL_API Render2D
{
public:
    struct RectVertex
    {
        Vector3 Position;
        Vector4 Color;
        Vector2 TexCoord;
        float   TexIndex;
        float   TilingFactor;
        int     Object;
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
        uint32_t RectCount = 0;

        uint32_t TotalVertexCount() const
        {
            return RectCount * 4;
        }

        uint32_t TotalIndexCount() const
        {
            return RectCount * 6;
        }
    };

    struct Data
    {
#ifdef __APPLE__
        static constexpr uint32_t MaxTextureSlots = 16;
#else
		static constexpr uint32_t MaxTextureSlots = 32;
#endif
		static constexpr uint32_t MaxRects    = 20000;
		static constexpr uint32_t MaxVertices = MaxRects * 4;
		static constexpr uint32_t MaxIndices  = MaxRects * 6;

        Ref<Texture> WhiteTexture;
		URef<DescriptorBuffer> textureDescriptorBuffer;

        uint32_t RectIndexCount = 0;
        std::vector<RectVertex> RectVertexBuffer;

        std::array<Ref<Texture>, MaxTextureSlots> ActiveTextures;
        uint32_t TextureSlotIndex = 1; // 0 = white texture

        Vector4 RectVertexPositions[4];

        Statistics Stats;

        RectVertex *pRectVertex = nullptr;
    };

public:
    static void Setup();

    static void Release();

    static void Setup(const Ref<RenderTarget> &renderTarget)
    {
        pipeline->Reconstruct(renderTarget);
    }

    static void Shutdown();

    static void Flush();

    static void StartBatch()
    {
        data.RectIndexCount   = 0;
        data.pRectVertex      = data.RectVertexBuffer.data(); // reset to start
        data.TextureSlotIndex = 1;
    }

    static void NextBatch()
    {
        Flush();
        StartBatch();
    }

    static void ResetStats()
    {
        CleanUpObject(&data.Stats);
    }

    static void BeginScene(const Camera &camera)
    {
        auto viewProjection = camera.ViewProjection();
        uniform->Update(sizeof(Matrix4), &viewProjection);
        NextBatch();
    }

    static void BeginScene(const Camera &camera, const Matrix4 &view)
    {
        auto viewProjection = camera.Projection() * Vector::Inverse(view);
        uniform->Update(sizeof(Matrix4), &viewProjection);
        NextBatch();
    }

    static void EndScene()
    {
        NextBatch();
    }

    static void DrawRect(const Matrix4 &transform, const Vector4 &color, int object = -1);

    static void DrawRect(const Matrix4 &transform, const Ref<Texture> &texture, float tilingFactor = 1.0f, const Vector4 &tintColor = Vector4(1.0f), int object = -1);

    static void DrawRect(const Vector2 &position, const Vector2 &size, const Vector4 &color)
    {
        DrawRect({ position.x, position.y, 0.0f }, size, color);
    }

    static void DrawRect(const Vector3 &position, const Vector2 &size, const Vector4 &color)
    {
        Matrix4 transform = Vector::Translate(position) *
            Vector::Scale({ size.x, size.y, 1.0f });
        DrawRect(transform, color);
    }

    static void DrawRect(const Vector2 &position, const Vector2 &size, const Ref<Texture> &texture, float tilingFactor = 1.0f, const Vector4 &tintColor = Vector4(1.0f))
    {
        DrawRect({ position.x, position.y, 0.0f }, size, texture, tilingFactor, tintColor);
    }

    static void DrawRect(const Vector3 &position, const Vector2 &size, const Ref<Texture> &texture, float tilingFactor = 1.0f, const Vector4 &tintColor = Vector4(1.0f))
    {
        Matrix4 transform = Vector::Translate(position) * Vector::Scale({ size.x, size.y, 1.0f });
        DrawRect(transform, texture, tilingFactor, tintColor);
    }

    static void DrawRotatedRect(const Vector2 &position, const Vector2 &size, float rotation, const Vector4 &color)
    {
        DrawRotatedRect({ position.x, position.y, 0.0f }, size, rotation, color);
    }

    static void DrawRotatedRect(const Vector3 &position, const Vector2 &size, float rotation, const Vector4 &color)
    {
        Matrix4 transform = Vector::Translate(position)
            * Vector::Rotate(rotation, { 0.0f, 0.0f, 1.0f })
            * Vector::Scale({ size.x, size.y, 1.0f });
        DrawRect(transform, color);
    }

    static void DrawRotatedRect(const Vector2 &position, const Vector2 &size, float rotation, const Ref<Texture> &texture, float tilingFactor = 1.0f, const Vector4 &tintColor = Vector4{ 1.0f })
    {
        DrawRotatedRect({ position.x, position.y, 0.0f }, size, rotation, texture, tilingFactor, tintColor);
    }

    static void DrawRotatedRect(const Vector3 &position, const Vector2 &size, float rotation, const Ref<Texture> &texture, float tilingFactor = 1.0f, const Vector4 &tintColor = Vector4{ 1.0f })
    {
        Matrix4 transform = Vector::Translate(position) * Vector::Rotate(rotation, { 0.0f, 0.0f, 1.0f }) * Vector::Scale({ size.x, size.y, 1.0f });
        DrawRect(transform, texture, tilingFactor, tintColor);
    }

    static void DrawSprite(const Matrix4 &transform, SpriteRendererComponent &src, int object)
    {
        DrawRect(transform, src.Sprite, src.TilingFactor, src.Color, object);
    }

public:
    static Data data;

    static Statistics Stats();

    static Ref<GraphicsPipeline> pipeline;

    static Ref<Buffer> uniform;

    static inline bool isTextureChanged = false;
};

}
