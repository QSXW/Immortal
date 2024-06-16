#pragma once

#include "Core.h"

#include "Camera.h"
#include "Scene/Component.h"
#include "Math/Vector.h"
#include "Graphics/LightGraphics.h"

#include <array>

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

#ifdef __APPLE__
    static constexpr uint32_t MaxTextureSlots = 16;
#else
    static constexpr uint32_t MaxTextureSlots = 32;
#endif
    static constexpr uint32_t MaxRects    = 20000;
    static constexpr uint32_t MaxVertices = MaxRects * 4;
    static constexpr uint32_t MaxIndices  = MaxRects * 6;

public:
	Render2D();

    ~Render2D();

    void Flush();

    void StartBatch();

    void NextBatch();

    void BeginScene(const Camera &camera);

    void EndScene();

    void DrawRect(const Matrix4 &transform, const Vector4 &color, int object = -1);

    void DrawRect(const Matrix4 &transform, const Ref<Texture> &texture, float tilingFactor = 1.0f, const Vector4 &tintColor = Vector4(1.0f), int object = -1);

    void DrawRect(const Vector2 &position, const Vector2 &size, const Vector4 &color)
    {
        DrawRect({ position.x, position.y, 0.0f }, size, color);
    }

    void DrawRect(const Vector3 &position, const Vector2 &size, const Vector4 &color)
    {
        Matrix4 transform = Vector::Translate(position) * Vector::Scale({ size.x, size.y, 1.0f });
        DrawRect(transform, color);
    }

    void DrawRect(const Vector2 &position, const Vector2 &size, const Ref<Texture> &texture, float tilingFactor = 1.0f, const Vector4 &tintColor = Vector4(1.0f))
    {
        DrawRect({ position.x, position.y, 0.0f }, size, texture, tilingFactor, tintColor);
    }

    void DrawRect(const Vector3 &position, const Vector2 &size, const Ref<Texture> &texture, float tilingFactor = 1.0f, const Vector4 &tintColor = Vector4(1.0f))
    {
        Matrix4 transform = Vector::Translate(position) * Vector::Scale({ size.x, size.y, 1.0f });
        DrawRect(transform, texture, tilingFactor, tintColor);
    }

    void DrawRotatedRect(const Vector2 &position, const Vector2 &size, float rotation, const Vector4 &color)
    {
        DrawRotatedRect({ position.x, position.y, 0.0f }, size, rotation, color);
    }

    void DrawRotatedRect(const Vector3 &position, const Vector2 &size, float rotation, const Vector4 &color)
    {
        Matrix4 transform = Vector::Translate(position)
            * Vector::Rotate(rotation, { 0.0f, 0.0f, 1.0f })
            * Vector::Scale({ size.x, size.y, 1.0f });
        DrawRect(transform, color);
    }

    void DrawRotatedRect(const Vector2 &position, const Vector2 &size, float rotation, const Ref<Texture> &texture, float tilingFactor = 1.0f, const Vector4 &tintColor = Vector4{ 1.0f })
    {
        DrawRotatedRect({ position.x, position.y, 0.0f }, size, rotation, texture, tilingFactor, tintColor);
    }

    void DrawRotatedRect(const Vector3 &position, const Vector2 &size, float rotation, const Ref<Texture> &texture, float tilingFactor = 1.0f, const Vector4 &tintColor = Vector4{ 1.0f })
    {
        Matrix4 transform = Vector::Translate(position) * Vector::Rotate(rotation, { 0.0f, 0.0f, 1.0f }) * Vector::Scale({ size.x, size.y, 1.0f });
        DrawRect(transform, texture, tilingFactor, tintColor);
    }

    void DrawSprite(const Matrix4 &transform, SpriteRendererComponent &src, int object)
    {
        DrawRect(transform, src.Sprite, src.TilingFactor, src.Color, object);
    }

public:
    Ref<GraphicsPipeline> pipeline;

    Matrix4 viewProjection;

    uint32_t rectIndexCount;

    URef<Buffer> vertexBuffer;

    URef<Buffer> indexBuffer;

    URef<DescriptorSet>descriptorSet;

    Sampler *sampler;
    URef<Sampler> linearSampler;
    URef<Sampler> pointSampler;

	std::array<Texture *, MaxTextureSlots> textures;

	uint32_t textureIndex;

	Vector4 RectVertexPositions[4];

	RectVertex *pRectVertex = nullptr;
};

}
