#include "impch.h"
#include "ObserverCamera.h"

#include "Math/Math.h"
#include "Framework/Input.h"

namespace Immortal
{
void ObserverCamera::UpdateViewMatrix()
{
    Matrix4 rotationMatrix    = Vector::Rotate(rotation);
    Matrix4 translationMatrix = Vector::Translate(position);

    if (type == Type::FirstPerson)
    {
        Camera::view = rotationMatrix * translationMatrix;
    }
    else
    {
        Camera::view = translationMatrix * rotationMatrix;
    }

    updated = true;
}

void ObserverCamera::OnUpdate(float deltaTime)
{
    if (Input::IsKeyPressed(KeyCode::A))
    {
        keys.Left = true;
    }
    if (Input::IsKeyPressed(KeyCode::D))
    {
        keys.Right = true;
    }
    if (Input::IsKeyPressed(KeyCode::W))
    {
        keys.Up = true;
    }
    if (Input::IsKeyPressed(KeyCode::S))
    {
        keys.Down = true;
    }

    if (Input::IsMouseButtonPressed(MouseCode::Left))
    {
        mouseButtons.Left = true;
    }
    if (Input::IsMouseButtonPressed(MouseCode::Right))
    {
        mouseButtons.Right = true;
    }
    if (Input::IsMouseButtonPressed(MouseCode::Middle))
    {
        mouseButtons.Middle = true;
    }

    Vector2 currentPos = Input::GetMousePosition();
    INT32 dx = (INT32)(mousePos.x - currentPos.x);
    INT32 dy = (INT32)(mousePos.y - currentPos.y);

    if (mouseButtons.Left)
    {
        rotation += Vector3(dy * rotationSpeed * deltaTime, -dx * rotationSpeed * deltaTime, 0.0f);
    }
    if (mouseButtons.Right)
    {
        zoom += dy * .005f * zoomSpeed * deltaTime;
        position += Vector3(-0.0f, 0.0f, zoom);
    }
    if (mouseButtons.Middle)
    {
        position += Vector3(-dx * 0.01f, -dy * 0.01f, 0.0f);
    }

    mouseButtons.Left   = false;
    mouseButtons.Right  = false;
    mouseButtons.Middle = false;
    mousePos.x = currentPos.x;
    mousePos.y = currentPos.y;

    this->Update(deltaTime);

    keys.Left  = false;
    keys.Right = false;
    keys.Up    = false;
    keys.Down  = false;
}

void ObserverCamera::Update(float deltaTime)
{
    updated = false;
    if (type == Type::FirstPerson)
    {
        if (Moving())
        {
            Vector3 front = Vector::Normalize(Vector3 {
                -Math::Cos(Vector::Radians(rotation.x)) * Math::Sin(Vector::Radians(rotation.y)),
                    Math::Sin(Vector::Radians(rotation.x)),
                    Math::Cos(Vector::Radians(rotation.x)) * Math::Cos(Vector::Radians(rotation.y))
            });

            float moveSpeed = deltaTime * translationSpeed;

            if (keys.Up)
            {
                position += front * moveSpeed;
            }
            if (keys.Down)
            {
                position -= front * moveSpeed;
            }
            if (keys.Left)
            {
                position -= Vector::Normalize(Vector::Cross(front, Vector3(0.0f, 1.0f, 0.0f))) * moveSpeed;
            }
            if (keys.Right)
            {
                position += Vector::Normalize(Vector::Cross(front, Vector3(0.0f, 1.0f, 0.0f))) * moveSpeed;
            }
        }
    }
        
    UpdateViewMatrix();
}

bool ObserverCamera::UpdateGamepad(Vector2 axisLeft, Vector2 axisRight, float deltaTime)
{
    bool changed = false;

    if (type == Type::FirstPerson)
    {
        // Use the common console thumbstick layout
        // Left = view, right = move
        const float deadZone = 0.0015f;
        const float range = 1.0f - deadZone;

        Vector3 front = Vector::Normalize(Vector3{
                -Math::Cos(Vector::Radians(rotation.x)) * Math::Sin(Vector::Radians(rotation.y)),
                    Math::Sin(Vector::Radians(rotation.x)),
                    Math::Cos(Vector::Radians(rotation.x)) * Math::Cos(Vector::Radians(rotation.y))
            });

        float moveSpeed = deltaTime * translationSpeed * 2.0f;
        float newRotationSpeed = deltaTime * rotationSpeed * 50.0f;

        // Move
        if (fabsf(axisLeft.y) > deadZone)
        {
            float pos = (fabsf(axisLeft.y) - deadZone) / range;
            position -= front * pos * ((axisLeft.y < 0.0f) ? -1.0f : 1.0f) * moveSpeed;
            changed = true;
        }
        if (fabsf(axisLeft.x) > deadZone)
        {
            float pos = (fabsf(axisLeft.x) - deadZone) / range;
            position += Vector::Normalize(Vector::Cross(front, Vector3(0.0f, 1.0f, 0.0f))) * pos * ((axisLeft.x < 0.0f) ? -1.0f : 1.0f) * moveSpeed;
            changed = true;
        }

        // Rotate
        if (fabsf(axisRight.x) > deadZone)
        {
            float pos = (fabsf(axisRight.x) - deadZone) / range;
            rotation.y += pos * ((axisRight.x < 0.0f) ? -1.0f : 1.0f) * newRotationSpeed;
            changed = true;
        }
        if (fabsf(axisRight.y) > deadZone)
        {
            float pos = (fabsf(axisRight.y) - deadZone) / range;
            rotation.x -= pos * ((axisRight.y < 0.0f) ? -1.0f : 1.0f) * newRotationSpeed;
            changed = true;
        }
    }
    else
    {
        // todo: move code from example base class for look-at
    }

    if (changed)
    {
        UpdateViewMatrix();
    }

    return changed;
}

inline bool ObserverCamera::Moving() const
{
    return keys.Left || keys.Right || keys.Up || keys.Down;
}

}
