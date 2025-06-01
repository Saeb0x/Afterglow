#pragma once

#include "Event.h"
#include <sstream>

namespace Afterglow
{
	class KeyEvent : public Event
	{
	public:
		inline virtual int GetCategoryFlags() const override { return (EventCategoryInput | EventCategoryKeyboard); }
		inline const int GetKeyCode() const { return m_KeyCode; }

	protected:
		KeyEvent(const int keyCode)
			: m_KeyCode(keyCode) 
		{
		}

		int m_KeyCode;
	};

	class KeyPressedEvent : public KeyEvent
	{
	public:
		KeyPressedEvent(const int keyCode, bool isRepeat) :
			KeyEvent(keyCode), m_IsRepeat(isRepeat) 
		{
		}

		inline virtual EventType GetType() const override { return EventType::KeyPressed; }
		inline virtual const char* GetName() const override { return "KeyPressedEvent"; }

		inline virtual std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyPressedEvent: " << m_KeyCode << " (repeat = " << m_IsRepeat << ")";
			return ss.str();
		}

		inline bool IsRepeat() const { return m_IsRepeat; }

	private:
		bool m_IsRepeat;
	};

	class KeyReleasedEvent : public KeyEvent
	{
	public:
		KeyReleasedEvent(const int keyCode)
			: KeyEvent(keyCode) 
		{
		}

		inline virtual EventType GetType() const override { return EventType::KeyReleased; }
		inline virtual const char* GetName() const override { return "KeyReleasedEvent"; }

		inline virtual std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyReleasedEvent: " << m_KeyCode;
			return ss.str();
		}
	};
}