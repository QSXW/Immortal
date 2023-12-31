#include "NativeInput.h"

namespace Immortal
{
 
static inline void GetClientCursorPosition(HWND handle, LPPOINT pos)
{
    if (::GetCursorPos(pos))
    {
        ::ScreenToClient(handle, pos);
    }
}

NativeInput::NativeInput(Window *window) :
    handle{ (HWND)(window->GetBackendHandle()) }
{
    !!Input::That ? throw Exception(SError::InvalidSingleton) : That = this;
    Clear();
}

NativeInput::~NativeInput()
{

}

void NativeInput::Clear()
{
    CleanUpObject(KeysDown, 0, sizeof(KeysDown));
	memset(MouseDown, 0, sizeof(MouseDown));
}

bool NativeInput::InternalIsKeyPressed(const KeyCode key)
{
	return KeysDown[uint32_t(key)];
}

bool NativeInput::InternalIsMouseButtonPressed(const MouseCode button)
{
	return MouseDown[uint32_t(button)];
}

Vec2 NativeInput::InternalGetMousePosition()
{
    POINT pos;
    GetClientCursorPosition(handle, &pos);

    return Vec2{
       (float)(pos.x),
       (float)(pos.y),
    };
}

float NativeInput::InternalGetMouseX()
{
    POINT pos;
    GetClientCursorPosition(handle, &pos);

    return (float)(pos.x);
}

float NativeInput::InternalGetMouseY()
{
    POINT pos;
    GetClientCursorPosition(handle, &pos);

    return (float)(pos.y);
}

}
