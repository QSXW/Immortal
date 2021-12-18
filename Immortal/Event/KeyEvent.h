#pragma once

#include "Event.h"
#include "Framework/KeyCodes.h"

namespace Immortal
{

class KeyEvent : public Event
{
public:
    KeyEvent(const int keyCode) :
        keyCode{ keyCode }
    {

    }

    KeyCode GetKeyCode() const
    {
        return ncast<KeyCode>(keyCode);
    }

    DEFINE_EVENT_CATEGORY(Category::Keyboard | Category::Input)
   
protected:
    int keyCode;
};

class KeyPressedEvent : public KeyEvent
{
public:
    KeyPressedEvent(const int keyCode, const uint16_t repeatCount) :
        KeyEvent{ keyCode },
        repeatCount{ repeatCount }
    {
    }

        
    uint16_t RepeatCount() const
    {
        return repeatCount;
    }

    std::string Stringify() const override
    {
        return std::string{ "KeyPressedEvent: " + std::to_string(keyCode) + " (" + std::to_string(repeatCount) + " repeats" };
    }

    DEFINE_EVENT_TYPE(KeyPressed)

private:
    uint16_t repeatCount;
};

class KeyReleasedEvent : public KeyEvent
{
public:
    KeyReleasedEvent(const int keyCode) :
        KeyEvent{ keyCode }
    {
    
    }

    std::string Stringify() const override
    {
        return std::string{ "KeyReleasedEvent: " + std::to_string(keyCode) };
    }

    DEFINE_EVENT_TYPE(KeyReleased)
};

class KeyTypedEvent : public KeyEvent
{
public:
    KeyTypedEvent(const int keyCode) :
        KeyEvent{ keyCode }
    {
        
    }

    std::string Stringify() const override
    {
        return std::string{ "KeyTypedEvent: " + std::to_string(keyCode) };
    }

    DEFINE_EVENT_TYPE(KeyTyped)
};

}
