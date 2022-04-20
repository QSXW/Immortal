#pragma once

#include "Core.h"
#include "KeyCodes.h"

#include "Math/Vector.h"

namespace Immortal
{

class IMMORTAL_API Input
{
public:
    static constexpr size_t MaxKeyCodes = 512;

    static constexpr size_t MaxMouseCodes = 7;

    Input() = default;

    virtual ~Input()
    {

    }

protected:
    virtual bool InternalIsKeyPressed(KeyCode key) = 0;

    virtual bool InternalIsMouseButtonPressed(MouseCode button) = 0;

    virtual Vector2 InternalGetMousePosition() = 0;

    virtual float InternalGetMouseX() = 0;

    virtual float InternalGetMouseY() = 0;

public:
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
    Input(const Input &) = delete;

    Input(Input &&) = delete;

    const Input &operator=(const Input &) = delete;

    const Input &operator=(Input &&) = delete;

protected:
    static Input *That;
};

}
