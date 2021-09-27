#pragma once

#include "impch.h"

#include "ImmortalCore.h"
#include "Event/Event.h"

namespace Immortal
{
class IMMORTAL_API Window
{
public:
    using EventCallbackFunc = std::function<void(Event&)>;

    struct Description
    {
    public:
        Description(const std::string &title = "Immortal Engine", UINT32 width = 1, UINT32 height = 1) :
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

        UINT32 Width;

        UINT32 Height;

        bool Vsync;
    };

public:
    virtual ~Window() { }

    virtual UINT32 Width() const = 0;

    virtual UINT32 Height() const = 0;

    virtual void SetEventCallback(const EventCallbackFunc& callback) = 0;

    virtual void* GetNativeWindow() const = 0;

    virtual void* PlatformNativeWindow() const = 0;

    virtual void Clear() {}

    virtual float Time() = 0;

    virtual void ProcessEvents() = 0;

    virtual float DpiFactor() const = 0;

    virtual void SetTitle(const std::string &title) = 0;

public:
    static std::unique_ptr<Window> Create(const Description &description = Description{});
};
}
