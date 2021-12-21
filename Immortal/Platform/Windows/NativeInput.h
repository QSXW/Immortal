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
        CleanUpObject(Input::KeysDown, 0, sizeof(Input::KeysDown));
        
        auto *i64 = rcast<uint64_t *>(Input::MouseDown);
        *i64 = 0;
    }
};

}
