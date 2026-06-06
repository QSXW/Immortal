#pragma once

#include "Core.h"
#include "Camera.h"
#include "Math/Vector.h"
#include <algorithm>

namespace Immortal
{

class IMMORTAL_API OrthographicCamera : public Camera
{
public:
    using Super = Camera;

public:
    OrthographicCamera() :
        Super{ ProjectionType::Orthographic }
    {
		SetClipPlanes(0.01f, 2.0f);
    }

    OrthographicCamera(const Vector2 &size) :
	    OrthographicCamera{}
    {
		SetViewportSize(size);
    }

    OrthographicCamera(float left, float right, float bottom, float top) :
        Camera{ Vector::Ortho(left, right, bottom, top, -1.0f, 1.0f) }
    {
        projectionType = ProjectionType::Orthographic;
        viewProjection = projection * view;
		SetClipPlanes(0.01f, 2.0f);
    }

    void SetProjection(float left, float right, float bottom, float top)
    {
        projection = Vector::Ortho(left, right, bottom, top, -1.0f, 1.0f);
        viewProjection = Super::ViewProjection();
		SetClipPlanes(0.01f, 2.0f);
    }

    void SetProjection(Matrix4 prj)
    {
        projection = prj;
        viewProjection = Super::ViewProjection();
    }

    void SetPosition(const Vector3 &pos)
    {
        position = pos;
        ReCalculateViewMatrix();
    }

    void Set(const Vector3 &pos, float rot)
    {
        position = pos;
        rotation = rot;
        ReCalculateViewMatrix();
    }

    float Rotation() const
    {
        return rotation;
    }

    const Vector3 &Position() const
    {
        return position;
    }

    void SetRotation(float other)
    {
        rotation = other;
        ReCalculateViewMatrix();
    }

    virtual Matrix4 ViewProjection() const override
    {
        return viewProjection;
    }

    float GetZoomLevel() const
    {
		return zoomLevel;
    }

    float GetZoomLevelTarget() const
    {
        return zoomLevelTarget;
    }

    float CalculateZoomLevelForScroll(float offsetY) const
    {
        float step = 8.0f;
        if (zoomLevelTarget < 0.09f)
        {
            step = 1.0f;
        }
        return std::max(zoomLevelTarget - offsetY * step * 0.01f, 0.00001f);
    }

    void SetZoomLevel(float value)
    {
        const float next = value > 0.0f ? value : 0.5f;
        zoomLevelTarget = std::max(next, 0.00001f);
    }

public:
    virtual void SetViewportSize(Vector2 size) override;

    virtual void OnUpdate(const float &deltaTime = Time::DeltaTime) override;

    virtual bool OnMouseScrolled(MouseScrolledEvent &e) override;

    void OnKeyCodeUpdate(const float &deltaTime = Time::DeltaTime);

private:
    void ReCalculateViewMatrix();

private:
    Matrix4 viewProjection;
    Vector3 position{ 0.0f };

    float aspectRatio    = 0.0f;
    float zoomLevel      = 0.5f;
    float translateSpeed = 50.0f;
    float rotation       = 0.0f;
    float rotateSpeed    = 180.0f;
    bool  rotated        = false;

    float zoomLevelTarget = 0.5f;
};

}
