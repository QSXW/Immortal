#pragma once

#include "ImmortalCore.h"
#include "Vector.h"
#include "KeyCodes.h"

namespace Immortal
{
class IMMORTAL_API Input
{
public:
    Input() = default;

    virtual bool InternalIsKeyPressed(KeyCode key)
    {
        return IsKeyPressed(key);
    }

    static bool IsKeyPressed(KeyCode key);

    static bool IsMouseButtonPressed(MouseCode button);

    static Vector::Vector2 GetMousePosition();

    static float GetMouseX();

    static float GetMouseY();

public:
    virtual ~Input() = default;

private:
    Input(const Input &) = delete;
    Input(Input &&) = delete;
    const Input &operator=(const Input &) = delete;
    const Input &operator=(Input &&) = delete;
};
}
