#pragma once

#include "Event.h"

namespace Afterglow
{
	class KeyEvent : public Event
	{
	public:
		inline int GetKeyCode() const { return m_KeyCode; }
	protected:
		KeyEvent(int keyCode) : m_KeyCode(keyCode) {}
		int m_KeyCode;
	};

	class KeyPressEvent : public KeyEvent
	{
	public:
		KeyPressEvent(int keyCode, int repeatCount) : KeyEvent(keyCode), m_RepeatCount(repeatCount) {}

		inline int GetRepeatCount() const { return m_RepeatCount; }

		DEFINE_EVENT_TYPE(KeyPress)
	private:
		int m_RepeatCount;
	};

	class KeyReleaseEvent : public KeyEvent
	{
	public:
		KeyReleaseEvent(int keyCode) : KeyEvent(keyCode) {}

		DEFINE_EVENT_TYPE(KeyRelease)
	};
}
