#pragma once

#include "Framework/Input.h"
#include "Framework/Window.h"

namespace Immortal
{

class IMMORTAL_API NativeInput : public Input
{
public:
    NativeInput(Window *window);

    virtual ~NativeInput();

    void Clear();

protected:
    virtual bool InternalIsKeyPressed(KeyCode key) override;

    virtual bool InternalIsMouseButtonPressed(MouseCode button) override;

    virtual Vector2 InternalGetMousePosition() override;

    virtual float InternalGetMouseX() override;

    virtual float InternalGetMouseY() override;

public:
    HWND handle{ nullptr };

    bool KeysDown[MaxKeyCodes];

    bool MouseDown[sizeof(int64_t)];

    bool Focus{ false };

    struct
    {
        bool Control;

        bool Shift;

        bool Alt;
    } Modifiers;
};

}
