#pragma once

#include "agpch.h"
#include "Event.h"

namespace Afterglow
{
	class MouseButtonEvent : public Event
	{
	public:
		inline int GetMouseButton() const { return m_MouseButton; }
	protected:
		MouseButtonEvent(int mouseButton) : m_MouseButton(mouseButton) {}
		int m_MouseButton;
	};

	class MouseButtonPressEvent : public MouseButtonEvent
	{
	public:
		MouseButtonPressEvent(int mouseButton) : MouseButtonEvent(mouseButton) {}

		inline std::string ToString() const override
		{
			std::stringstream ss;
			ss << "Mouse Button Press Event: " << m_MouseButton << std::endl;
			return ss.str();
		}
		DEFINE_EVENT_TYPE(MouseButtonPress)
	};

	class MouseButtonReleaseEvent : public MouseButtonEvent
	{
	public:
		MouseButtonReleaseEvent(int mouseButton) : MouseButtonEvent(mouseButton) {}

		inline std::string ToString() const override
		{
			std::stringstream ss;
			ss << "Mouse Button Release Event: " << m_MouseButton << std::endl;
			return ss.str();
		}
		DEFINE_EVENT_TYPE(MouseButtonRelease)
	};

	class MouseMoveEvent : public Event
	{
	public:
		MouseMoveEvent(float x, float y) : m_MouseX(x), m_MouseY(y) {}

		inline float GetMouseX() const { return m_MouseX; }
		inline float GetMouseY() const { return m_MouseY; }

		DEFINE_EVENT_TYPE(MouseMove)

		inline std::string ToString() const override
		{
			std::stringstream ss;
			ss << "Mouse Move Event: (" << m_MouseX << " ," << m_MouseY << ")";
			return ss.str();
		}
	private:
		float m_MouseX, m_MouseY;
	};
}