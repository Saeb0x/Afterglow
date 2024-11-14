#pragma once

#include "Event.h"

namespace Afterglow
{
	class WindowResizeEvent : public Event
	{
	public:
		WindowResizeEvent(int width, int height) : m_Width(width), m_Height(height) {}

		inline std::string ToString() const override
		{
			std::stringstream ss;
			ss << "Window Resize Event: (" << m_Width << ", " << m_Height << ")\n";
			return ss.str();
		}

		DEFINE_EVENT_TYPE(WindowResize)
	private:
		int m_Width, m_Height;
	};


	class WindowCloseEvent : public Event
	{
	public:
		inline std::string ToString() const override
		{
			std::stringstream ss;
			ss << "Window Close Event\n";
			return ss.str();
		}

		DEFINE_EVENT_TYPE(WindowClose)
	};
}