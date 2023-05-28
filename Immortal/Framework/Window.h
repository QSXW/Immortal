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
} \

class IMMORTAL_API Window
{
public:
    using EventCallbackFunc = std::function<void(Event&)>;

    enum Type : int
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

    struct Description
    {
    public:
        Description(const std::string &title = "Immortal Engine", uint32_t width = 1, uint32_t height = 1) :
            Title{ title }, Width{ width }, Height{ height }, Vsync{ true }, EventCallback{ nullptr }
        {

        }

        Description(Description &&other) :
            Title{ other.Title }, Width{ other.Width }, Height{ other.Height }, Vsync{ other.Vsync }, EventCallback{ other.EventCallback }
        {

        }

        Description(const Description &other) :
            Title{ other.Title }, Width{ other.Width }, Height{ other.Height }, Vsync{ other.Vsync }, EventCallback{ other.EventCallback }
        {

        }

        Description &operator =(const Description &other)
        {
            if (this != &other)
            {
                Title         = other.Title;
                Width         = other.Width;
                Height        = other.Height;
                Vsync         = other.Vsync;
                EventCallback = other.EventCallback;
            }

            return *this;
        }

        Description &operator =(Description &&other)
        {
            if (this != &other)
            {
                Title         = std::move(std::move(other.Title));
                Width         = std::move(other.Width);
                Height        = std::move(other.Height);
                Vsync         = std::move(other.Vsync);
                EventCallback = std::move(other.EventCallback);
            }

            return *this;
        }

    public:
        EventCallbackFunc EventCallback;

        std::string Title;

        uint32_t Width;

        uint32_t Height;

        bool Vsync;
    };

public:
    virtual ~Window() { }

    virtual uint32_t Width() const = 0;

    virtual uint32_t Height() const = 0;

    virtual void SetEventCallback(const EventCallbackFunc& callback) = 0;

    virtual Anonymous Primitive() const = 0;

    virtual void ProcessEvents() = 0;

    virtual void Clear() const {}

    virtual float Time() const { return .0f; }

    virtual float DpiFactor() const { return .0f;  }

    virtual void SetTitle(const std::string &title) {}

    virtual void Show() {}

    virtual void SetIcon(const std::string &filepath) {}

    Type GetType() const
    {
        return type;
    }

protected:
    Type type = Type::None;

public:
    static Window *CreateInstance(const Description &description = Description{});
};

}
