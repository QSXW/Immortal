#pragma once

#include "ImmortalCore.h"

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

    OrthographicCamera& Camera() { return mCamera; }
    const OrthographicCamera& Camera() const { return mCamera; }

    float ZoomLevel() const { return mZoomLevel; }
    void SetZoomLevel(float level = 1.0f) { mZoomLevel = level; }
    float AspectRatio() const { return mAspectRatio; }

    void Reset(Vector::Vector3 &position = Vector::Vector3(0.0f), float rotation = 0.0f);

private:
    bool OnMouseScrolled(MouseScrolledEvent &e);
    bool OnWindowResized(WindowResizeEvent &e);

private:
    float mAspectRatio;
    float mZoomLevel = 1.0f;
    OrthographicCamera mCamera;

    bool mRotation;

    Vector::Vector3 mCameraPosition{ 0.0f, 0.0f, 0.0f };
    float mCameraRotation = 0.0f;
    float mCameraTranslationSpeed = 5.0f;
    float mCameraRotationSpeed = 180.0f;
};
}
