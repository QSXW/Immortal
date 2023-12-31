#pragma once

#include "Event/Event.h"
#include "KeyCodes.h"

namespace Immortal
{

class MouseMoveEvent : public Event
{
public:
    MouseMoveEvent(const float x, const float y) :
        mouseX{ x },
        mouseY{ y }
    {

    }

    float GetX() const
    {
        return mouseX;
    }

    float GetY() const
    {
        return mouseY;
    }

    std::string Stringify() const override
    {
        return std::string{ "MouseMovedEvent: " + std::to_string(mouseX) + ", " + std::to_string(mouseY) };
    }

    DEFINE_EVENT_TYPE(MouseMoved)
    DEFINE_EVENT_CATEGORY(Category::Mouse | Category::Input)

private:
    float mouseX;
    float mouseY;
};

class MouseScrolledEvent : public Event
{
public:
    MouseScrolledEvent(const float offsetX, const float offsetY) :
        offsetX{ offsetX },
        offsetY{ offsetY }
    {

    }

    float GetOffsetX() const
    {
        return offsetX;
    }

    float GetOffsetY() const
    {
        return offsetY;
    }

    std::string Stringify() const override
    {
        return std::string{ "MouseScrolledEvent: " + std::to_string(offsetX) + ", " + std::to_string(offsetY)  };
    }

    DEFINE_EVENT_TYPE(MouseScrolled)
    DEFINE_EVENT_CATEGORY(Category::Mouse | Category::Input)

private:
    float offsetX;
    float offsetY;
};

class MouseButtonEvent : public Event
{
public:
    MouseButtonEvent(const MouseCode button) :
        button{ button }
    {

    }

    MouseCode GetMouseButton() const
    {
        return button;
    }

    DEFINE_EVENT_CATEGORY(Category::Mouse | Category::Input | Category::MouseButton)

protected:
    MouseCode button;
};

class MouseButtonPressedEvent : public MouseButtonEvent
{
public:
    MouseButtonPressedEvent(const MouseCode button) :
        MouseButtonEvent{ button }
    {

    }

    std::string Stringify() const override
    {
        return std::string{ "MouseButtonPressedEvent: " + std::to_string((int)(button)) };
    }

    DEFINE_EVENT_TYPE(MouseButtonPressed)
};

class MouseButtonReleasedEvent : public MouseButtonEvent
{
public:
    MouseButtonReleasedEvent(const MouseCode button) :
        MouseButtonEvent{ button }
    {

    }

    std::string Stringify() const override
    {
        return std::string{ "MouseButtonReleasedEvent: " + std::to_string((int)(button)) };
    }

    DEFINE_EVENT_TYPE(MouseButtonReleased)
};

}
