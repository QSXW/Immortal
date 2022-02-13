#pragma once

#include "Core.h"

#include <string>
#include <functional>

namespace Immortal
{

#define DEFINE_EVENT_TYPE(T) \
static Event::Type GetStaticType() \
{ \
    return Event::Type::T; \
} \
\
virtual Event::Type GetType() const override \
{ \
    return GetStaticType(); \
} \
\
virtual std::string Name() const override \
{\
    return #T; \
}

#define DEFINE_EVENT_CATEGORY(category) \
virtual Event::Category GetCategoryFlags() const override \
{ \
    return category; \
}

class IMMORTAL_API Event
{
public:
    enum class Type
    {
        None = 0,
        WindowClose,
        WindowResize,
        WindowFocus,
        WindowLostFocus,
        WindowMoved,
        AppTick,
        AppUpdate,
        AppRender,
        KeyPressed,
        KeyReleased,
        KeyTyped,
        MouseButtonPressed,
        MouseButtonReleased,
        MouseMoved,
        MouseScrolled,
        MaxCount
    };

    enum class Category
    {
        None = 0,
        Application = BIT(0),
        Input       = BIT(1),
        Keyboard    = BIT(2),
        Mouse       = BIT(3),
        MouseButton = BIT(4)
    };

public:
    virtual Type GetType() const = 0;

    virtual std::string Name() const = 0;

    virtual Category GetCategoryFlags() const = 0;

    virtual std::string Stringify() const
    {
        return Name();
    }

    bool IsInCategory(Category category);

public:
    bool Handled = false;
};

SL_DEFINE_BITWISE_OPERATION(Event::Category, int32_t)

inline bool Event::IsInCategory(Event::Category category)
{
    return GetCategoryFlags() & category;
}

class EventDispatcher
{
public:
    EventDispatcher(Event &event)
        : e(event)
    {

    }

    template <class T, class F>
    bool Dispatch(const F &func)
    {
        if (e.GetType() == T::GetStaticType())
        {
            e.Handled |= func(static_cast<T&>(e));
            return true;
        }
        return false;
    }

private:
    Event &e;
};

template <class T>
class EventSink
{
public:
    using Handle = T*;
    using Func   = bool(T::*)(Event&);

public:
    EventSink() :
        that{ nullptr }
    {
        CleanUp();
    }

    EventSink(Handle other) :
        that{ other }
    {
        CleanUp();
    }

    void CleanUp()
    {
        for (auto &&e : pool)
        {
            e = nullptr;
        }
    }

    template <class F>
    void Listen(const F &func, Event::Type type)
    {
        pool[U64(type)] = rcast<Func>(func);
    }

    void Dispatch(Event &e)
    {
        auto func = pool[U64(e.GetType())];
        if (!func)
        {
            return;
        }
        e.Handled |= std::bind(func, that, std::placeholders::_1)(e);
    }

private:
    Handle that;

    std::array<Func, U64(Event::Type::MaxCount)> pool;
};

inline std::ostream& operator<<(std::ostream &os, const Event &e)
{
    return os << e.Stringify();
}

}
