#include "agpch.h"
#include "Application.h"

#include "Afterglow/Core/Log.h"

#include "Afterglow/Events/EventBus.h"
#include "Afterglow/Events/EventDispatcher.h"
#include "Afterglow/Events/MouseEvents.h"

namespace Afterglow
{
	#define BIND_FN(x) std::bind(&x, this, std::placeholders::_1)

	Application::Application()
	{
		m_Window = std::unique_ptr<Window>(Window::Create());

		// Subscribe to all events, then dispatch them internally.
		m_EventSubscriptionIndex = EventBus::GetInstance().Subscribe<Afterglow::Event>(BIND_FN(Application::OnEvent));
	}

	Application::~Application()
	{
		// We need to unsubscribe based on the index we got when we subscribed. This WILL change in the future :)
		EventBus::GetInstance().Unsubscribe<Event>(m_EventSubscriptionIndex);
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
		EventDispatcher disp(e);
		disp.Dispatch<WindowCloseEvent>(BIND_FN(Application::OnWindowClose));
		disp.Dispatch<WindowResizeEvent>(BIND_FN(Application::OnWindowResize));
		
		disp.Dispatch<MouseMovedEvent>([](MouseMovedEvent& e)
			{
				AG_LOG_INFO("{0}", e.ToString());
				return true;
			}
		);
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;
		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		AG_LOG_INFO("{0}", e.ToString());
		return true;
	}
}