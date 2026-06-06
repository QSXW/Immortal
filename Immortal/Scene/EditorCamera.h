#pragma once

#include "Core.h"
#include "Event/MouseEvent.h"
#include "Render/Camera.h"
#include "Math/Vector.h"

namespace Immortal
{

class EditorCamera : public Camera
{
public:
    EditorCamera()
	{
		SetClipPlanes(0.1f, 1000.0f);
	}

    EditorCamera(float fov, float width, float height, float zNear, float zFar);

    EditorCamera(const Matrix4 &projection);

    void Focus(const Vector3 &focusPoint);
	
    virtual void OnUpdate(const float &deltaTime = Time::DeltaTime) override;

    virtual void OnEvent(Event &e) override;

    void SetViewportSize(Vector2 size);

	/** Read/write editor orbit for scene persistence (optional JSON block). */
	void ExportOrbitSnapshot(Vector3 &outFocal, float &outDist, float &outPitchDeg, float &outYawDeg, float &outFovDeg) const;

	void ImportOrbitSnapshot(const Vector3 &focal, float dist, float pitchDeg, float yawDeg, float fovDeg, float zNear, float zFar);

    Vector3 UpDirection();
    Vector3 RightDirection();
    Vector3 ForwardDirection();

    const float &Distance() const
    {
        return distance;
    }

    const Vector3 &Position() const
    {
        return position;
    }

    Quaternion Orientation() const;

    float Pitch() const
    {
        return pitch;
    }

    float Yaw() const
    {
        return yaw;
    }

private:
    constexpr float RotateSpeed() const
    {
        return 0.8f;
    }

    void UpdateView();
    bool OnMouseMoved(MouseMoveEvent &e);
    bool OnMouseButtonPressed(MouseButtonPressedEvent &e);
    bool OnMouseScroll(MouseScrolledEvent &e);

    void MousePan(const Vector::Vector2 &delta);
    void MouseRotate(const Vector::Vector2 &delta);
    void MouseZoom(float delta);

    Vector3 CalculatePosition();

    Vector2 PanSpeed() const;

    float ZoomSpeed() const;

private:
    Vector2 viewportSize = { 1.0f, 1.0f };
    Vector3 position     = { 0.0f, 0.0f, 0.0f };
    Vector3 rotation     = { 0.0f, 0.0f, 0.0f };
    Vector3 focalPoint   = { 0.0f, 0.0f, 0.0f };

    Vector2 targetPanDelta     = { 0.0f, 0.0f };
    Vector2 targetRotateDelta   = { 0.0f, 0.0f };
    float   targetZoomDelta     = 0.0f;
    Vector2 lastMouseForOrbit   = { 0.0f, 0.0f };
    bool    hasLastMouseForOrbit = false;

    float FOV               = 45.0f;
    float distance          = 8.0f;
    float pitch             = 0.0f;
    float yaw               = 0.0f;
    float minFocusDistance  = 100.0f;

    bool  panning  = true;
    bool  rotating = true;
};

}
