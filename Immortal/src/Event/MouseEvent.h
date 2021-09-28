#pragma once

#include "Event/Event.h"
#include "Framework/KeyCodes.h"

namespace Immortal
{

class MouseMoveEvent : public Event
{
public:
    MouseMoveEvent(const float x, const float y)
        : mouseX(x), mouseY(y) { }

    float GetX() const { return mouseX; }
    float GetY() const { return mouseY; }

    std::string Stringify() const override
    {
        std::string s;
        s += "MouseMovedEvent: " + std::to_string(mouseX) + ", " + std::to_string(mouseY);
        return s;
    }

    EVENT_CLASS_TYPE(MouseMoved)
    EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

private:
    float mouseX;
    float mouseY;
};

class MouseScrolledEvent : public Event
{
public:
    MouseScrolledEvent(const float xOffset, const float yOffset)
        : xOffset(xOffset), yOffset(yOffset) {}

    float GetXOffset() const { return xOffset; }
    float GetYOffset() const { return yOffset; }

    std::string Stringify() const override
    {
        std::string s;
        s += "MouseScrolledEvent: " + std::to_string(xOffset) + ", " + std::to_string(yOffset);
        return s;
    }

    EVENT_CLASS_TYPE(MouseScrolled)
    EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
private:
    float xOffset;
    float yOffset;
};

class MouseButtonEvent : public Event
{
public:
    MouseCode GetMouseButton() const { return button; }

    EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput | EventCategoryMouseButton)

protected:
    MouseButtonEvent(const MouseCode button)
        : button(button) {}

    MouseCode button;
};

class MouseButtonPressedEvent : public MouseButtonEvent
{
public:
    MouseButtonPressedEvent(const MouseCode button)
        : MouseButtonEvent(button) {}

    std::string Stringify() const override
    {
        std::string s;
        s += "MouseButtonPressedEvent: " + std::to_string((int)button);
        return s;
    }

    EVENT_CLASS_TYPE(MouseButtonPressed)
};

class MouseButtonReleasedEvent : public MouseButtonEvent
{
public:
    MouseButtonReleasedEvent(const MouseCode button)
        : MouseButtonEvent(button) {}

    std::string Stringify() const override
    {
        std::string s;
        s += "MouseButtonReleasedEvent: " + std::to_string((int)button);
        return s;
    }

    EVENT_CLASS_TYPE(MouseButtonReleased)
};

}