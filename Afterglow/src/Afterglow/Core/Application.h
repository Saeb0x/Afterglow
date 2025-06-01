#pragma once

#include <memory>

#include "Window.h"

#include "Afterglow/Events/IEventSubscriber.h"
#include "Afterglow/Events/WindowEvents.h"
#include "Afterglow/Events/MouseEvents.h"
#include "Afterglow/Events/KeyboardEvents.h"

namespace Afterglow
{
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
