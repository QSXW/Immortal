#pragma once

#include "Core.h"
#include "Camera.h"
#include "Math/Vector.h"

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
        
    }

    OrthographicCamera(float left, float right, float bottom, float top) :
        Camera{ Vector::Ortho(left, right, bottom, top, -1.0f, 1.0f) }
    {
        projectionType = ProjectionType::Orthographic;
        viewProjection = projection * view;
    }

    void SetProjection(float left, float right, float bottom, float top)
    {
        projection = Vector::Ortho(left, right, bottom, top, -1.0f, 1.0f);
        viewProjection = Super::ViewProjection();
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

    virtual void SetViewportSize(Vector2 size) override;

    virtual void OnUpdate() override;

    virtual bool OnMouseScrolled(MouseScrolledEvent &e) override;

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
};

}
