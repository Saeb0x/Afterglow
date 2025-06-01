#include "agpch.h"
#include "Application.h"

#include "Base.h"

#include "Afterglow/Events/EventDispatcher.h"

namespace Afterglow
{
	#define BIND_FN(x) std::bind(&x, this, std::placeholders::_1)

	Application::Application()
	{
		m_Window = std::unique_ptr<Window>(Window::Create());

		Subscribe<WindowCloseEvent>(BIND_FN(Application::OnWindowClose));
		Subscribe<MouseMovedEvent>(BIND_FN(Application::OnMouseMoved));
	}

	Application::~Application()
	{
	}
	
	void Application::Run()
	{
		while (m_Running)
		{
			m_Window->OnUpdate();
		}
	}

	void Application::OnEvent(Event& e)
	{
	}

	bool Application::OnWindowClose(const WindowCloseEvent& e)
	{
		m_Running = false;
		return true;
	}

	bool Application::OnMouseMoved(const MouseMovedEvent& e)
	{
		AG_LOG_INFO(e.ToString());
		return true;
	}
}