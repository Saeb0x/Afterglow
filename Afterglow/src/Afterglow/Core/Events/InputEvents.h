#pragma once

#include "Event.h"

#include <string>
#include <sstream>

namespace Afterglow
{
	class KeyEvent : public Event
	{
	public:
		int GetCategoryFlags() const override { return (EventCategoryInput | EventCategoryKeyboard); }
		int GetKeyCode() const { return m_KeyCode; }

	protected:
		KeyEvent(int keyCode)
			: m_KeyCode(keyCode)
		{
		}

		int m_KeyCode;
	};

	class KeyPressedEvent : public KeyEvent
	{
	public:
		KeyPressedEvent(int keyCode, bool repeated) :
			KeyEvent(keyCode), b_Repeat(repeated)
		{
		}
		~KeyPressedEvent() override = default;

		static EventType GetStaticType() { return EventType::KeyPressed; }
		EventType GetType() const override { return GetStaticType(); }
		const char* GetName() const override { return "KeyPressedEvent"; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "Key Pressed Event: " << m_KeyCode << " (Repeat = " << b_Repeat << ")";
			return ss.str();
		}

		bool IsRepeat() const { return b_Repeat; }

	private:
		bool b_Repeat;
	};

	class KeyReleasedEvent : public KeyEvent
	{
	public:
		KeyReleasedEvent(int keyCode)
			: KeyEvent(keyCode)
		{
		}
		~KeyReleasedEvent() override = default;

		static EventType GetStaticType() { return EventType::KeyReleased; }
		EventType GetType() const override { return GetStaticType(); }
		const char* GetName() const override { return "KeyReleasedEvent"; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "Key Released Event: " << m_KeyCode;
			return ss.str();
		}
	};

	class MouseButtonEvent : public Event
	{
	public:
		int GetCategoryFlags() const override { return (EventCategoryInput | EventCategoryMouse); }
		int GetMouseButton() const { return m_MouseButton; }

	protected:
		MouseButtonEvent(int mouseButton)
			: m_MouseButton(mouseButton)
		{
		}

		int m_MouseButton;
	};

	class MouseButtonPressedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonPressedEvent(int mouseButton)
			: MouseButtonEvent(mouseButton)
		{
		}
		~MouseButtonPressedEvent() override = default;

		static EventType GetStaticType() { return EventType::MouseButtonPressed; }
		EventType GetType() const override { return GetStaticType(); }
		const char* GetName() const override { return "MouseButtonPressedEvent"; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "Mouse Button Pressed Event: " << m_MouseButton;
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
		~MouseButtonReleasedEvent() override = default;

		static EventType GetStaticType() { return EventType::MouseButtonReleased; }
		EventType GetType() const override { return GetStaticType(); }
		const char* GetName() const override { return "MouseButtonReleasedEvent"; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "Mouse Button Released Event: " << m_MouseButton;
			return ss.str();
		}
	};

	class MouseMovedEvent : public Event
	{
	public:
		MouseMovedEvent(float mouseX, float mouseY)
			: m_MouseX(mouseX), m_MouseY(mouseY)
		{
		}
		~MouseMovedEvent() override = default;

		static EventType GetStaticType() { return EventType::MouseMoved; }
		EventType GetType() const override { return GetStaticType(); }
		const char* GetName() const override { return "MouseMovedEvent"; }
		int GetCategoryFlags() const override { return (EventCategoryInput | EventCategoryMouse); }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "Mouse Moved Event: " << m_MouseX << ", " << m_MouseY;
			return ss.str();
		}

		float GetMouseX() const { return m_MouseX; }
		float GetMouseY() const { return m_MouseY; }

	private:
		float m_MouseX, m_MouseY;
	};

	class MouseScrolledEvent : public Event
	{
	public:
		MouseScrolledEvent(float xOffset, float yOffset)
			: m_XOffset(xOffset), m_YOffset(yOffset)
		{
		}
		~MouseScrolledEvent() override = default;

		static EventType GetStaticType() { return EventType::MouseScrolled; }
		EventType GetType() const override { return GetStaticType(); }
		const char* GetName() const override { return "MouseScrolledEvent"; }
		int GetCategoryFlags() const override { return (EventCategoryInput | EventCategoryMouse); }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "Mouse Scrolled Event: " << m_XOffset << ", " << m_YOffset;
			return ss.str();
		}

		float GetXOffset() const { return m_XOffset; }
		float GetYOffset() const { return m_YOffset; }

	private:
		float m_XOffset, m_YOffset;
	};
}