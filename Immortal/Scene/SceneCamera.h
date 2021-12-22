#pragma once

#include "ImmortalCore.h"
#include "Render/Camera.h"

namespace Immortal
{

class SceneCamera : public Camera
{
public:
    SceneCamera();

    virtual ~SceneCamera();

    void SetPerspective(float verticalFOV, float nearClip = 1.0f, float farClip = 10000.0f);

    void SetOrthographic(float size, float nearClip = -1.0f, float farClip = 1.0f);

    void SetViewportSize(float width, float height);

    void SetViewportSize(const Vector2 &size)
    {
        SetViewportSize(size.x, size.y);
    }

    void SetPerspectiveVerticalFOV(float verticalFov)
    { 
        perspectiveFOV = Vector::Radians(verticalFov);
    }

    float PerspectiveVerticalFOV() const
    {
        return Vector::Degrees(perspectiveFOV);
    }

    void SetPerspectiveNearClip(float nearClip)
    {
        perspectiveNear = nearClip;
    }

    float &PerspectiveNearClip()
    {
        return perspectiveNear;
    }

    void SetPerspectiveFarClip(float farClip)
    {
        perspectiveFar = farClip;
    }

    float &PerspectiveFarClip()
    {
        return perspectiveFar;
    }

    void SetOrthographicSize(float size)
    {
        orthographicSize = size;
    }

    float OrthographicSize() const
    { 
        return orthographicSize;
    }

    float &OrthographicSize()
    { 
        return orthographicSize;
    }

    void SetOrthographicNearClip(float nearClip)
    { 
        orthographicNear = nearClip;
    }

    float &OrthographicNearClip()
    {
        return orthographicNear;
    }

    void SetOrthographicFarClip(float farClip)
    {
        orthographicFar = farClip;
    }

    float &OrthographicFarClip()
    {
        return orthographicFar;
    }

    void SetProjectionType(ProjectionType type)
    {
        this->type = type;
    }

    ProjectionType Type() const
    { 
        return type;
    }

    void SetTransform(const Matrix4 &transform)
    {
        view = Vector::Inverse(transform);
    }

protected:
    ProjectionType type = ProjectionType::Perspective;

    float perspectiveFOV{ Vector::Radians(90.0f) };
    float perspectiveNear{ 0.1f };
    float perspectiveFar{ 1000.0f };
              
    float orthographicSize{ 10.0f };
    float orthographicNear{ -1.0f };
    float orthographicFar{ 1.0f };
};
}
