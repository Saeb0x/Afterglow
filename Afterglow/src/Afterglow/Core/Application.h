#pragma once

#include <memory>

#include "Window.h"

#include "Afterglow/Events/Event.h"
#include "Afterglow/Events/WindowEvents.h"

namespace Afterglow
{
	class Application
	{
	public:
		Application();
		virtual ~Application();

		virtual void Run();
		
		void OnEvent(Event& e);

	private:
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);

	private:
		bool m_Running = true;
		size_t m_EventSubscriptionIndex;
		std::unique_ptr<Window> m_Window;
	};
}
