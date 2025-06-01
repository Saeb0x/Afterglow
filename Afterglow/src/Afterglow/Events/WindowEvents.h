#pragma once

#include "Event.h"
#include <sstream>

namespace Afterglow
{
	class WindowResizeEvent : public Event
	{
	public:
		WindowResizeEvent(unsigned int width, unsigned int height)
			: m_Width(width), m_Height(height) 
		{
		}

		inline virtual EventType GetType() const override { return EventType::WindowResize; }
		inline virtual const char* GetName() const override { return "WindowResizeEvent"; }
		inline virtual int GetCategoryFlags() const override { return EventCategoryWindow; }

		inline virtual std::string ToString() const override
		{
			std::stringstream ss;
			ss << "WindowResizeEvent: " << m_Width << ", " << m_Height;
			return ss.str();
		}

		unsigned int GetWidth() const { return m_Width; }
		unsigned int GetHeight() const { return m_Height; }
	private:
		unsigned int m_Width, m_Height;
	};


	class WindowCloseEvent : public Event
	{
	public:
		WindowCloseEvent() = default;

		inline virtual EventType GetType() const override { return EventType::WindowClose; }
		inline virtual const char* GetName() const override { return "WindowCloseEvent"; }
		inline virtual int GetCategoryFlags() const override { return EventCategoryWindow; }

		inline virtual std::string ToString() const override
		{
			std::stringstream ss;
			ss << "WindowCloseEvent";
			return ss.str();
		}
	};
}