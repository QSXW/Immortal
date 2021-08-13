#include "CubeController.h"
#include <Immortal/Core/Application.h>
#include <Immortal/Core/Input.h>

namespace Immortal
{
    void CubeController::OnStart()
    {

    }

    void CubeController::OnDestroy()
    {

    }

    void CubeController::OnUpdate()
    {
        constexpr float speed = 1.0f;
        float deltaTime = Application::Time.DeltaTime();

        auto &transform = GetComponent<TransformComponent>();

        if (Input::IsKeyPressed(KeyCode::A))
        {
            transform.Position.x -= speed * deltaTime;
        }
        if (Input::IsKeyPressed(KeyCode::D))
        {
            transform.Position.x += speed * deltaTime;
        }
        if (Input::IsKeyPressed(KeyCode::W))
        {
            transform.Position.y += speed * deltaTime;
        }
        if (Input::IsKeyPressed(KeyCode::S))
        {
            transform.Position.y -= speed * deltaTime;
        }
    }
}

extern "C" void* GetCubeController()
{
    return new Immortal::CubeController();
}

