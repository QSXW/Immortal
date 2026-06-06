#pragma once

#include "Core.h"
#include "Render/Camera.h"

namespace Immortal
{

class SceneCamera : public Camera
{
public:
    SceneCamera();

    virtual ~SceneCamera();

    void SetPerspective(float verticalFOV, float nearClip = 0.1f, float farClip = 10000.0f);

    void SetOrthographic(float size, float nearClip = 0.0f, float farClip = 1.0f);

    virtual void SetViewportSize(float width, float height);

    bool OnMouseScrolled(MouseScrolledEvent &e);

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

    float PerspectiveNearClip() const
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

    float PerspectiveFarClip() const
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

    float OrthographicNearClip() const
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

    float OrthographicFarClip() const
	{
		return orthographicFar;
	}

    void SetProjectionType(ProjectionType type)
    {
        projectionType = type;
    }

    ProjectionType GetType() const
    {
        return projectionType;
    }

	/** Copies current perspective / orthographic clip distances to Camera::SetClipPlanes (shadow fitting). */
	void RefreshCameraClipPlanes();

    void SetTransform(const Matrix4 &transform)
    {
        view = Vector::Inverse(transform);
    }

protected:
    float perspectiveFOV{ Vector::Radians(90.0f) };
    float perspectiveNear{ 0.1f };
    float perspectiveFar{ 1000.0f };

    float orthographicSize{ 10.0f };
    float orthographicNear{ -1.0f };
    float orthographicFar{ 1.0f };
};
}
