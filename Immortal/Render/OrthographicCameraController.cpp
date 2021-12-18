#include "OrthographicCameraController.h"

namespace Immortal
{

OrthographicCameraController::OrthographicCameraController(float aspectRatio, bool rotation) :
    aspectRatio{ aspectRatio },
    camera{ -aspectRatio * zoomLevel, aspectRatio * zoomLevel, -zoomLevel, zoomLevel },
    rotation{ rotation }
{

}

OrthographicCameraController::OrthographicCameraController(float width, float height, bool rotation) :
    aspectRatio{ width / height },
    camera{ -aspectRatio * zoomLevel, aspectRatio * zoomLevel, -zoomLevel, zoomLevel },
    rotation{ rotation }
{
    
}

void OrthographicCameraController::OnUpdate(float deltaTime)
{
    if (Input::IsKeyPressed(KeyCode::A))
    {
        cameraPosition.x -= cos(Vector::Radians(cameraRotation)) * cameraTranslationSpeed * deltaTime;
        cameraPosition.y -= sin(Vector::Radians(cameraRotation)) * cameraTranslationSpeed * deltaTime;
    }
    else if (Input::IsKeyPressed(KeyCode::D))
    {
        cameraPosition.x += cos(Vector::Radians(cameraRotation)) * cameraTranslationSpeed * deltaTime;
        cameraPosition.y += sin(Vector::Radians(cameraRotation)) * cameraTranslationSpeed * deltaTime;
    }

    if (Input::IsKeyPressed(KeyCode::W))
    {
        cameraPosition.x += -sin(Vector::Radians(cameraRotation)) * cameraTranslationSpeed * deltaTime;
        cameraPosition.y += cos(Vector::Radians(cameraRotation)) * cameraTranslationSpeed * deltaTime;
    }
    else if (Input::IsKeyPressed(KeyCode::S))
    {
        cameraPosition.x -= -sin(Vector::Radians(cameraRotation)) * cameraTranslationSpeed * deltaTime;
        cameraPosition.y -= cos(Vector::Radians(cameraRotation)) * cameraTranslationSpeed * deltaTime;
    }

    if (rotation)
    {
        if (Input::IsKeyPressed(KeyCode::Q))
        {
            cameraRotation += cameraRotationSpeed * deltaTime;
        }
        if (Input::IsKeyPressed(KeyCode::E))
        {
            cameraRotation -= cameraRotationSpeed * deltaTime;
        }

        if (cameraRotation > 180.0f)
        {
            cameraRotation -= 360.0f;
        } 
        else if (cameraRotation <= -180.0f)
        {
            cameraRotation += 360.0f;
        }

        camera.SetRotation(cameraRotation);
    }

    camera.setPosition(cameraPosition);
    cameraTranslationSpeed = zoomLevel;
}

void OrthographicCameraController::OnEvent(Event &e)
{
    EventDispatcher dispatcher(e);
    dispatcher.Dispatch<MouseScrolledEvent>(std::bind(&OrthographicCameraController::OnMouseScrolled, this, std::placeholders::_1));
    dispatcher.Dispatch<WindowResizeEvent>(std::bind(&OrthographicCameraController::OnWindowResized, this, std::placeholders::_1));
}

void OrthographicCameraController::Reset(Vector::Vector3 &position, float rotation)
{
    cameraPosition = position;
    cameraRotation = rotation;
    camera.Set(cameraPosition, cameraRotation);
}

void OrthographicCameraController::Resize(float width, float height)
{
    aspectRatio = width / height;
    camera.SetProjection(-aspectRatio * zoomLevel, aspectRatio * zoomLevel, -zoomLevel, zoomLevel);
}

bool OrthographicCameraController::OnMouseScrolled(MouseScrolledEvent &e)
{
    zoomLevel -= e.GetOffsetY() * 0.25f;
    zoomLevel = std::max(zoomLevel, 0.1f);
    camera.SetProjection(-aspectRatio * zoomLevel, aspectRatio * zoomLevel, -zoomLevel, zoomLevel);
    return false;
}

bool OrthographicCameraController::OnWindowResized(WindowResizeEvent &e)
{
    Resize((float)e.Width(), (float)e.Height());
    return false;
}

}
