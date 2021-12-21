#pragma once

#include "ImmortalCore.h"
#include "Vector.h"
#include "KeyCodes.h"

namespace Immortal
{

class IMMORTAL_API Input
{
public:
    static constexpr size_t MaxKeyCodes = 512;

    static constexpr size_t MaxMouseCodes = 7;

public:
    Input() = default;

    virtual ~Input()
    {

    }

    virtual bool InternalIsKeyPressed(KeyCode key) = 0;

    virtual bool InternalIsMouseButtonPressed(MouseCode button) = 0;

    virtual Vector2 InternalGetMousePosition() = 0;

    virtual float InternalGetMouseX() = 0;

    virtual float InternalGetMouseY() = 0;

    static bool IsKeyPressed(KeyCode key)
    {
        return That->InternalIsKeyPressed(key);
    }

    static bool IsMouseButtonPressed(MouseCode button)
    {
        return That->InternalIsMouseButtonPressed(button);
    }

    static Vector2 GetMousePosition()
    {
        return That->InternalGetMousePosition();
    }

    static float GetMouseX()
    {
        return GetMousePosition().x;
    }

    static float GetMouseY()
    {
        return GetMousePosition().y;
    }

public:
    bool KeysDown[MaxKeyCodes];

    bool MouseDown[sizeof(int64_t)];

    Input(const Input &) = delete;

    Input(Input &&) = delete;

    const Input &operator=(const Input &) = delete;

    const Input &operator=(Input &&) = delete;

protected:
    static Input *That;
};

}
