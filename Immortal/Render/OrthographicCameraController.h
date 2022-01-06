#pragma once

#include "Core.h"

#include "OrthographicCamera.h"

#include "Framework/Timer.h"

#include "Event/ApplicationEvent.h"
#include "Event/MouseEvent.h"
#include "Framework/Input.h"

namespace Immortal {

class OrthographicCameraController
{
public:
    OrthographicCameraController(float aspectRatio, bool rotation = false);

    OrthographicCameraController(float width, float height, bool rotation = false);

    void OnUpdate(float deltaTime);

    void OnEvent(Event& e);

    void Resize(float width, float height);

    OrthographicCamera &Camera()
    { 
        return camera;
    }

    operator const OrthographicCamera&() const
    {
        return camera;
    }

    const OrthographicCamera &Camera() const
    { 
        return camera;
    }

    float ZoomLevel() const
    { 
        return zoomLevel;
    }

    void SetZoomLevel(float level = 1.0f)
    {
        zoomLevel = level;
    }

    float AspectRatio() const
    { 
        return aspectRatio;
    }

    void Reset(Vector3 &position = Vector3{ 0.0f }, float rotation = 0.0f);

private:
    bool OnMouseScrolled(MouseScrolledEvent &e);
    bool OnWindowResized(WindowResizeEvent &e);

private:
    OrthographicCamera camera;

    Vector3 cameraPosition{ 0.0f, 0.0f, 0.0f };

    bool rotation;

    float aspectRatio;

    float zoomLevel              = 1.0f;

    float cameraRotation         = 0.0f;

    float cameraRotationSpeed    = 180.0f;

    float cameraTranslationSpeed = 5.0f;
};
}
