#include "GLFWInput.h"
#include <GLFW/glfw3.h>

namespace Immortal
{
    
GLFWInput::GLFWInput(Window *window) :
    windowHandle{ rcast<GLFWwindow *>(window->GetNativeWindow()) }
{
    !!Input::That ? throw Exception(SError::InvalidSingleton) : That = this;
}

GLFWInput::~GLFWInput()
{

}

bool GLFWInput::InternalIsKeyPressed(const KeyCode key)
{
    auto state = glfwGetKey(windowHandle, static_cast<int32_t>(key));
    return state == GLFW_PRESS || state == GLFW_REPEAT;
}

bool GLFWInput::InternalIsMouseButtonPressed(const MouseCode button)
{
    auto state = glfwGetMouseButton(windowHandle, static_cast<int32_t>(button));
    return state == GLFW_PRESS;
}

Vector2 GLFWInput::InternalGetMousePosition()
{
    double xpos, ypos;
    glfwGetCursorPos(windowHandle, &xpos, &ypos);

    return Vector2{ ncast<float>(xpos), ncast<float>(ypos) };
}

float GLFWInput::InternalGetMouseX()
{
    return GetMousePosition().x;
}

float GLFWInput::InternalGetMouseY()
{
    return GetMousePosition().y;
}

}
