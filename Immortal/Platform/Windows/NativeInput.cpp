#include "NativeInput.h"

namespace Immortal
{
 
NativeInput::NativeInput(Window *window)
{
    !!Input::That ? throw Exception(SError::InvalidSingleton) : That = this;
    CleanUpInputs();
}

NativeInput::~NativeInput()
{

}

bool NativeInput::InternalIsKeyPressed(const KeyCode key)
{
    return KeysDown[U32(key)];
}

bool NativeInput::InternalIsMouseButtonPressed(const MouseCode button)
{
    return MouseDown[U32(button)];
}

Vector2 NativeInput::InternalGetMousePosition()
{
    return Vector2{ 0, 0 };
}

float NativeInput::InternalGetMouseX()
{
    return 0.0f;
}

float NativeInput::InternalGetMouseY()
{
    return 0.0f;
}

}
