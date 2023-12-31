#pragma once

#include "Input.h"
#include "GLFWWindow.h"

namespace Immortal
{

class IMMORTAL_API GLFWInput : public Input
{
public:
    GLFWInput(Window *window);

    virtual ~GLFWInput();

protected:
    virtual bool InternalIsKeyPressed(KeyCode key) override;

    virtual bool InternalIsMouseButtonPressed(MouseCode button) override;

    virtual Vec2 InternalGetMousePosition() override;

    virtual float InternalGetMouseX() override;

    virtual float InternalGetMouseY() override;

private:
    GLFWwindow *windowHandle;
};

}
