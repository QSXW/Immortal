#include "NativeInput.h"

namespace Immortal
{
 
NativeInput::NativeInput(Window *window)
{
    !!Input::That ? throw Exception(SError::InvalidSingleton) : That = this;
}

NativeInput::~NativeInput()
{

}

bool NativeInput::InternalIsKeyPressed(const KeyCode key)
{
    return Input::KeysDown[U32(key)];
}

bool NativeInput::InternalIsMouseButtonPressed(const MouseCode button)
{
    return false;
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
