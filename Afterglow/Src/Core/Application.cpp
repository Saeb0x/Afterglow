#include "Application.h"
#include "Log.h"
#include "Window.h"
#include "Input.h"

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
			// Poll window events (updates Input internally via WindowProc)
			m_Window->Update();

			// Client game loop
			OnUpdate();

			// Update Input state (swap current/previous)
			Input::Update();
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