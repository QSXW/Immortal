#pragma once

#include "Event.h"
#include "Framework/KeyCodes.h"

namespace Immortal {

	class KeyEvent : public Event
	{
	public:
		int GetKeyCode() const { return m_keyCode; }
		EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)
		
	/* Protected constructor and should not be able to make this class */
	protected:
		KeyEvent(const int keyCode)
			: m_keyCode(keyCode) { }

		int m_keyCode;
	};

	class KeyPressedEvent : public KeyEvent
	{
	public:
		KeyPressedEvent(const int keyCode, const uint16_t repeatCount)
			: KeyEvent(keyCode), m_repeatCount(repeatCount) { }
		
		uint16_t RepeatCount() const { return m_repeatCount; }

		std::string Stringify() const override
		{
			std::string s;
			s.append("KeyPressedEvent: ").append(std::to_string(m_keyCode)).append(" (").append(std::to_string(m_repeatCount)).append(" repeats");
			return s;
		}

		EVENT_CLASS_TYPE(KeyPressed)
	private:
		uint16_t m_repeatCount;
	};

	class KeyReleasedEvent : public KeyEvent
	{
	public:
		KeyReleasedEvent(const int keyCode)
			: KeyEvent(keyCode) { }

		std::string Stringify() const override
		{
			std::string s;
			s.append("KeyReleasedEvent: ").append(std::to_string(m_keyCode));
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
			s.append("KeyTypedEvent: ").append(std::to_string(m_keyCode));
			return s;
		}
		EVENT_CLASS_TYPE(KeyTyped)
	};

}