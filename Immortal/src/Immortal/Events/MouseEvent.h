#pragma once

#include "Immortal/Events/Event.h"
#include "Immortal/Core/KeyCodes.h"

namespace Immortal {

	class MouseMoveEvent : public Event
	{
	public:
		MouseMoveEvent(const float x, const float y)
			: mMouseX(x), mMouseY(y) { }

		float GetX() const NOEXCEPT { return mMouseX; }
		float GetY() const NOEXCEPT { return mMouseY; }

		std::string ToString() const override
		{
			std::string s;
			s += "MouseMovedEvent: " + std::to_string(mMouseX) + ", " + std::to_string(mMouseY);
			return s;
		}

		EVENT_CLASS_TYPE(MouseMoved)
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

	private:
		float mMouseX;
		float mMouseY;
	};

	class MouseScrolledEvent : public Event
	{
	public:
		MouseScrolledEvent(const float xOffset, const float yOffset)
			: mXOffset(xOffset), mYOffset(yOffset) {}

		float GetXOffset() const NOEXCEPT { return mXOffset; }
		float GetYOffset() const NOEXCEPT { return mYOffset; }

		std::string ToString() const override
		{
			std::string s;
			s += "MouseScrolledEvent: " + std::to_string(mXOffset) + ", " + std::to_string(mYOffset);
			return s;
		}

		EVENT_CLASS_TYPE(MouseScrolled)
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
	private:
		float mXOffset;
		float mYOffset;
	};

	class MouseButtonEvent : public Event
	{
	public:
		MouseCode GetMouseButton() const { return mButton; }

		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput | EventCategoryMouseButton)

	protected:
		MouseButtonEvent(const MouseCode button)
			: mButton(button) {}

		MouseCode mButton;
	};

	class MouseButtonPressedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonPressedEvent(const MouseCode button)
			: MouseButtonEvent(button) {}

		std::string ToString() const override
		{
			std::string s;
			s += "MouseButtonPressedEvent: " + std::to_string((int)mButton);
			return s;
		}

		EVENT_CLASS_TYPE(MouseButtonPressed)
	};

	class MouseButtonReleasedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonReleasedEvent(const MouseCode button)
			: MouseButtonEvent(button) {}

		std::string ToString() const override
		{
			std::string s;
			s += "MouseButtonReleasedEvent: " + std::to_string((int)mButton);
			return s;
		}

		EVENT_CLASS_TYPE(MouseButtonReleased)
	};

}