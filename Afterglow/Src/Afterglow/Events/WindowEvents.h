#pragma once

#include "Event.h"

namespace Afterglow
{
	class WindowResizeEvent : public Event
	{
	public:
		WindowResizeEvent(int width, int height) : m_Width(width), m_Height(height) {}

		DEFINE_EVENT_TYPE(WindowResize)
	private:
		int m_Width, m_Height;
	};


	class WindowCloseEvent : public Event
	{
	public:
		DEFINE_EVENT_TYPE(WindowClose)
	};
}