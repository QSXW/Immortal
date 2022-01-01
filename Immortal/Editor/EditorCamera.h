#pragma once

#include "ImmortalCore.h"
#include "Event/MouseEvent.h"
#include "Render/Camera.h"

namespace Immortal
{

class EditorCamera : public Camera
{
public:
    EditorCamera() = default;

    EditorCamera(const Matrix4 &projection);

    void Focus(const Vector3 &focusPoint);
    void OnUpdate();
    void OnEvent(Event &e);

    void SetViewportSize(const Vector::Vector2 &size);

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

    Vector2 initialPosition   = { 0.0f, 0.0f };
    Vector3 initialFocalPoint = { 0.0f, 0.0f, 0.0f };
    Vector3 initialRotation   = { 0.0f, 0.0f, 0.0f };

    float FOV               = 45.0f;
    float distance          = 8.0f;
    float pitch             = 0.0f;
    float yaw               = 0.0f;
    float minFocusDistance  = 100.0f;

    bool  panning  = true;
    bool  rotating = true;
};

}
