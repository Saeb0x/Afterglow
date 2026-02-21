#include "Application.h"
#include "Log.h"
#include "Window.h"

namespace Afterglow
{
	Application::Application()
		: b_Running(true)
	{
	}

	void Application::Run()
	{
		WindowConfig config = GetWindowConfig();
		m_Window = Window::Create(config);

		OnInit();

		while (b_Running && !m_Window->ShouldClose())
		{
			m_Window->Update();

			OnUpdate();
		}

		OnShutdown();
	}

	LoggerConfig Application::GetLoggerConfig()
	{
		return LoggerConfig();
	}

	WindowConfig Application::GetWindowConfig()
	{
		return WindowConfig();
	}
}