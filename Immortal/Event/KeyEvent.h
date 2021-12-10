#pragma once

#include "Event.h"
#include "Framework/KeyCodes.h"

namespace Immortal
{

class KeyEvent : public Event
{
public:
    int GetKeyCode() const { return keyCode; }
    EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)

protected:
    KeyEvent(const int keyCode)
        : keyCode(keyCode) { }

    int keyCode;
};

class KeyPressedEvent : public KeyEvent
{
public:
    KeyPressedEvent(const int keyCode, const uint16_t repeatCount)
        : KeyEvent(keyCode), repeatCount(repeatCount) { }
        
    uint16_t RepeatCount() const { return repeatCount; }

    std::string Stringify() const override
    {
        std::string s;
        s.append("KeyPressedEvent: ").append(std::to_string(keyCode)).append(" (").append(std::to_string(repeatCount)).append(" repeats");
        return s;
    }

    EVENT_CLASS_TYPE(KeyPressed)
private:
    uint16_t repeatCount;
};

class KeyReleasedEvent : public KeyEvent
{
public:
    KeyReleasedEvent(const int keyCode)
        : KeyEvent(keyCode) { }

    std::string Stringify() const override
    {
        std::string s;
        s.append("KeyReleasedEvent: ").append(std::to_string(keyCode));
        return s;
    }
    EVENT_CLASS_TYPE(KeyReleased)
};

class KeyTypedEvent : public KeyEvent
{
public:
    KeyTypedEvent(const int keyCode)
        : KeyEvent(keyCode) { }

    std::string Stringify() const override
    {
        std::string s;
        s.append("KeyTypedEvent: ").append(std::to_string(keyCode));
        return s;
    }
    EVENT_CLASS_TYPE(KeyTyped)
};

}
