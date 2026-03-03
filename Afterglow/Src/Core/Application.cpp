#include "Application.h"
#include "Log.h"
#include "Window.h"
#include "Input.h"
#include "GraphicsContext.h"
#include "Renderer/Renderer.h"
#include "Renderer/RenderCommand.h"

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

		Renderer::Init();

		OnInit();

		while (b_Running && !m_Window->ShouldClose())
		{
			// Poll window events (updates Input internally via WindowProc)
			m_Window->Update();

			RenderCommand::SetClearColor(0.1f, 0.2f, 0.3f, 1.0f);
			RenderCommand::Clear();

			// Client game loop
			OnUpdate();

			m_Window->GetContext().SwapBuffers();

			// Update Input state (swap current/previous)
			Input::Update();
		}

		OnShutdown();
		Renderer::Shutdown();
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