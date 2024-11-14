#include "agpch.h"
#include "Application.h"

namespace Afterglow
{
	#define BIND_EVENT_FUNC(x) std::bind(&Application::x, this, std::placeholders::_1)

	Application::Application()
	{
		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(BIND_EVENT_FUNC(OnEvent));
	}

	Application::~Application()
	{
	}

	void Application::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);

		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FUNC(OnWindowsClose));
		dispatcher.Dispatch<MouseMoveEvent>(BIND_EVENT_FUNC(OnMouseMove));
	}

	bool Application::OnMouseMove(MouseMoveEvent& event)
	{
		AG_TRACE("{0}", event.ToString());
		return true;
	}

	bool Application::OnWindowsClose(WindowCloseEvent& event)
	{
		AG_TRACE("{0}", event.ToString());
		m_Running = false;
		return true;
	}

	void Application::Run()
	{
		while (m_Running)
		{
			m_Window->OnUpdate();
		}
	}
}
