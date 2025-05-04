#pragma once

#include <memory>

#include "Window.h"
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

#pragma region Events
	private:
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);
#pragma endregion

	private:
		bool m_Running = true;
		std::unique_ptr<Window> m_Window;

		size_t m_WindowCloseListener;
	};
}
