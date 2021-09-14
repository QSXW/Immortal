#pragma once

#include "ImmortalCore.h"

#include <string>
#include <functional>

namespace Immortal {

	enum class EventType
	{
		None = 0,
		WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
		AppTick, AppUpdate, AppRender,
		KeyPressed, KeyReleased, KeyTyped,
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
	};

	enum EventCategory
	{
		None						= 0,
		EventCategoryApplication	= BIT(0),
		EventCategoryInput			= BIT(1),
		EventCategoryKeyboard		= BIT(2),
		EventCategoryMouse			= BIT(3),
		EventCategoryMouseButton	= BIT(4)
	};

#define EVENT_CLASS_TYPE(T) \
	static EventType GetStaticType() { return EventType::T; }\
	virtual EventType Type() const override { return GetStaticType(); }\
	virtual const char *Name() const override { return #T; }

#define EVENT_CLASS_CATEGORY(category) \
	virtual int GetCategoryFlags() const override { return category; }

	class IMMORTAL_API Event
	{
	public:
		virtual EventType Type() const = 0;
		virtual const char *Name() const = 0;
		virtual int GetCategoryFlags() const = 0;
		virtual std::string Stringify() const { return Name(); }

		inline bool IsInCategory(EventCategory category)
		{
			return  GetCategoryFlags() & static_cast<int>(category);
		}

	public:
		bool Handled = false;
	};

	class EventDispatcher
	{
	public:
		EventDispatcher(Event &event)
			: mEvent(event)
		{

		}

		template <class T, class F>
		bool Dispatch(const F& func)
		{
			if (mEvent.Type() == T::GetStaticType())
			{
				mEvent.Handled |= func(static_cast<T&>(mEvent));
				return true;
			}
			return false;
		}

	private:
		Event& mEvent;
	};

	inline std::ostream& operator<<(std::ostream &os, const Event &e)
	{
		return os << e.Stringify();
	}

}
