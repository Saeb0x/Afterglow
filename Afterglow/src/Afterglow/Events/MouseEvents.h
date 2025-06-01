#pragma once

#include "Event.h"
#include <sstream>

namespace Afterglow
{
	class MouseButtonEvent : public Event
	{
	public:
		inline virtual int GetCategoryFlags() const override { return (EventCategoryInput | EventCategoryMouse); }

		const int GetMouseButton() const { return m_MouseButton; }
	protected:
		MouseButtonEvent(const int mouseButton)
			: m_MouseButton(mouseButton) 
		{
		}

		int m_MouseButton;
	};

	class MouseButtonPressedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonPressedEvent(const int mouseButton)
			: MouseButtonEvent(mouseButton) 
		{
		}

		inline virtual EventType GetType() const override { return EventType::MouseButtonPressed; }
		inline virtual const char* GetName() const override { return "MouseButtonPressedEvent"; }

		inline virtual std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonPressedEvent: " << m_MouseButton;
			return ss.str();
		}
	};

	class MouseButtonReleasedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonReleasedEvent(const int mouseButton)
			: MouseButtonEvent(mouseButton) 
		{
		}

		inline virtual EventType GetType() const override { return EventType::MouseButtonReleased; }
		inline virtual const char* GetName() const override { return "MouseButtonReleasedEvent"; }

		inline virtual std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonReleasedEvent: " << m_MouseButton;
			return ss.str();
		}
	};


	class MouseMovedEvent : public Event
	{
	public:
		MouseMovedEvent(const float mouseX, const float mouseY) 
			: m_MouseX(mouseX), m_MouseY(mouseY) 
		{
		}
		
		inline virtual EventType GetType() const override { return EventType::MouseMoved; }
		inline virtual const char* GetName() const override { return "MouseMovedEvent"; }
		inline virtual int GetCategoryFlags() const override { return (EventCategoryInput | EventCategoryMouse); }

		inline virtual std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseMovedEvent: " << m_MouseX << ", " << m_MouseY;
			return ss.str();
		}

		const float GetMouseX() const { return m_MouseX; }
		const float GetMouseY() const { return m_MouseY; }

	private:
		float m_MouseX, m_MouseY;
	};

	class MouseScrolledEvent : public Event
	{
	public:
		MouseScrolledEvent(const float xOffset, const float yOffset)
			: m_XOffset(xOffset), m_YOffset(yOffset) 
		{
		}

		inline virtual EventType GetType() const override { return EventType::MouseScrolled; }
		inline virtual const char* GetName() const override { return "MouseScrolledEvent"; }
		inline virtual int GetCategoryFlags() const override { return (EventCategoryInput | EventCategoryMouse); }

		inline virtual std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseScrolledEvent: " << m_XOffset << ", " << m_YOffset;
			return ss.str();
		}

		float GetXOffset() const { return m_XOffset; }
		float GetYOffset() const { return m_YOffset; }
		
	private:
		float m_XOffset, m_YOffset;
	};

}