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

    virtual bool InternalIsKeyPressed(KeyCode key) override;

    virtual bool InternalIsMouseButtonPressed(MouseCode button) override;

    virtual Vector2 InternalGetMousePosition() override;

    virtual float InternalGetMouseX() override;

    virtual float InternalGetMouseY() override;

    void CleanUpInputs()
    {
        CleanUpObject(KeysDown, 0, sizeof(KeysDown));

        auto *i64 = rcast<uint64_t *>(MouseDown);
        *i64 = 0;
    }

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
