#pragma once

#include "Core.h"
#include "Event/Event.h"

namespace Immortal
{

#define DEFINE_WINDOW_TYPE(T) \
static Window::Type GetStaticType() \
{ \
    return Window::Type::T; \
} \
\
virtual Window::Type GetType() const override \
{ \
    return GetStaticType(); \
}

enum WindowType : int
{
	None = 0,
	Wayland,
	X11,
	XCB,
	Cocoa,
	Win32,
	GLFW,
	Headless = None
};

class IMMORTAL_API Window
{
public:
    using EventCallbackFunc = std::function<void(Event&)>;

    using Type = WindowType;

public:
	virtual ~Window() = default;

    virtual uint32_t GetWidth() const = 0;

    virtual uint32_t GetHeight() const = 0;

    virtual void SetEventCallback(const EventCallbackFunc& callback) = 0;

    virtual Anonymous GetBackendHandle() const = 0;

    virtual Anonymous GetPlatformSpecificHandle() const = 0;

    virtual void ProcessEvents() = 0;

    virtual void SetTitle(const std::string &title) = 0;

    virtual void Show() = 0;

    virtual void SetIcon(const std::string &filepath) = 0;

public:
    Type GetType() const
    {
        return type;
    }

protected:
    Type type = Type::None;

public:
	static Window *CreateInstance(Anonymous handle, WindowType type);

	static Window *CreateInstance(const std::string &title, uint32_t width, uint32_t height, WindowType type = WindowType::None);
};

}
