#include "impch.h"

#include "WindowsInput.h"
#include "Framework/Application.h"
#include <GLFW/glfw3.h>

namespace Immortal
{

bool Input::IsKeyPressed(const KeyCode key)
{
    auto* window = static_cast<GLFWwindow*>(Application::App()->GetWindow().GetNativeWindow());
    auto state = glfwGetKey(window, static_cast<int32_t>(key));
    return state == GLFW_PRESS || state == GLFW_REPEAT;
}

bool Input::IsMouseButtonPressed(const MouseCode button)
{
    auto* window = static_cast<GLFWwindow*>(Application::App()->GetWindow().GetNativeWindow());
    auto state = glfwGetMouseButton(window, static_cast<int32_t>(button));
    return state == GLFW_PRESS;
}

Vector::Vector2 Input::GetMousePosition()
{
    auto* window = static_cast<GLFWwindow*>(Application::App()->GetWindow().GetNativeWindow());
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    return { (float)xpos, (float)ypos };
}

float Input::GetMouseX()
{
    return GetMousePosition().x;
}

float Input::GetMouseY()
{
    return GetMousePosition().y;
}

}
