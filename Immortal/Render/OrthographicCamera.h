#pragma once

#include "ImmortalCore.h"
#include "Camera.h"

namespace Immortal
{

class IMMORTAL_API OrthographicCamera : public Camera
{
public:
    using Super = Camera;

public:
    OrthographicCamera() = default;

    OrthographicCamera(float left, float right, float bottom, float top)
        : Camera(Vector::Ortho(left, right, bottom, top, -1.0f, 1.0f))
    {
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

    void setPosition(const Vector3 &pos)
    {
        position = pos;
        ReCalculateViewMatrix();
    }
        
    void Set(const Vector::Vector3 &pos, float rot)
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

    virtual Matrix4 ViewProjection() const
    {
        return viewProjection;
    }

private:
    void ReCalculateViewMatrix();

private:
    Matrix4 viewProjection;
    Vector3 position{ 0.0f, 0.0f, 0.0f };
    float   rotation{ 0.0f };
};

}

