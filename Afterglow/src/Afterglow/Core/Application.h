#pragma once

#include <memory>

#include "Window.h"
#include "Afterglow/Events/IEventSubscriber.h"

namespace Afterglow
{
	class WindowCloseEvent;
	class MouseMovedEvent;

	class Application : public IEventSubscriber
	{
	public:
		Application();
		virtual ~Application();

		virtual void Run();
		
		void OnEvent(Event& e);

	private:
		bool OnWindowClose(const WindowCloseEvent& e);
		bool OnMouseMoved(const MouseMovedEvent& e);

	private:
		bool m_Running = true;
		std::unique_ptr<Window> m_Window;
	};
}
