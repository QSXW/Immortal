#include "OrthographicCamera.h"

#include "Framework/Input.h"
#include "Framework/Timer.h"

namespace Immortal
{

void OrthographicCamera::SetViewportSize(Vector2 size)
{
    aspectRatio = size.x / size.y;
    SetProjection(-aspectRatio * zoomLevel, aspectRatio * zoomLevel, -zoomLevel, zoomLevel);
}

void OrthographicCamera::OnUpdate()
{
    float deltaTime = Time::DeltaTime;

    if (Input::IsKeyPressed(KeyCode::A))
    {
        position.x -= cos(Vector::Radians(rotation)) * translateSpeed * deltaTime;
        position.y -= sin(Vector::Radians(rotation)) * translateSpeed * deltaTime;
    }
    else if (Input::IsKeyPressed(KeyCode::D))
    {
        position.x += cos(Vector::Radians(rotation)) * translateSpeed * deltaTime;
        position.y += sin(Vector::Radians(rotation)) * translateSpeed * deltaTime;
    }

    if (Input::IsKeyPressed(KeyCode::W))
    {
        position.x += -sin(Vector::Radians(rotation)) * translateSpeed * deltaTime;
        position.y += cos(Vector::Radians(rotation)) * translateSpeed * deltaTime;
    }
    else if (Input::IsKeyPressed(KeyCode::S))
    {
        position.x -= -sin(Vector::Radians(rotation)) * translateSpeed * deltaTime;
        position.y -= cos(Vector::Radians(rotation)) * translateSpeed * deltaTime;
    }

    if (rotated)
    {
        if (Input::IsKeyPressed(KeyCode::Q))
        {
            rotation += rotateSpeed * deltaTime;
        }
        if (Input::IsKeyPressed(KeyCode::E))
        {
            rotation -= rotateSpeed * deltaTime;
        }

        if (rotation > 180.0f)
        {
            rotation -= 360.0f;
        }
        else if (rotation <= -180.0f)
        {
            rotation += 360.0f;
        }

        SetRotation(rotation);
    }

    SetPosition(position);
    translateSpeed = zoomLevel;
}

bool OrthographicCamera::OnMouseScrolled(MouseScrolledEvent & e)
{
    zoomLevel -= e.GetOffsetY() * 20.0f * Time::DeltaTime;
    zoomLevel = std::max(zoomLevel, 0.01f);
    SetProjection(-aspectRatio * zoomLevel, aspectRatio * zoomLevel, -zoomLevel, zoomLevel);
    return false;
}

void OrthographicCamera::ReCalculateViewMatrix()
{
    Matrix4 transform = Vector::Translate(position) * Vector::Rotate(rotation, Vector3(0, 0, 1));

    view = Vector::Inverse(transform);
    viewProjection = projection * view;
}

}
