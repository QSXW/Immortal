#pragma once

#include "Event.h"

namespace Immortal
{

class IMMORTAL_API WindowResizeEvent : public Event
{
public:
    WindowResizeEvent(uint32_t width, uint32_t height) :
        width{ width },
        height{ height }
    {
    
    }

    uint32_t Width() const
    {
        return width;
    }

    uint32_t Height() const
    {
        return height;
    }

    std::string Stringify() const override
    {
        return std::string{ "WindowResizeEvent: " + std::to_string(width) + ", " + std::to_string(height) };
    }

    DEFINE_EVENT_TYPE(WindowResize);
    DEFINE_EVENT_CATEGORY(Category::Application);

private:
    uint32_t width;
    uint32_t height;
};

class IMMORTAL_API WindowCloseEvent : public Event
{
public:
    WindowCloseEvent() = default;

    DEFINE_EVENT_TYPE(WindowClose);
    DEFINE_EVENT_CATEGORY(Category::Application);
};

class AppTickEvent : public Event
{
public:
    AppTickEvent() = default;

    DEFINE_EVENT_TYPE(AppTick);
    DEFINE_EVENT_CATEGORY(Category::Application);
};

class AppUpdateEvent : public Event
{
public:
    AppUpdateEvent() = default;

    DEFINE_EVENT_TYPE(AppUpdate);
    DEFINE_EVENT_CATEGORY(Category::Application);
};

class AppRenderEvent : public Event
{
public:
    AppRenderEvent() = default;

    DEFINE_EVENT_TYPE(AppRender);
    DEFINE_EVENT_CATEGORY(Category::Application);
};

}
