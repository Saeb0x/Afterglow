#include "agpch.h"
#include "Application.h"

#include "Base.h"
#include "Afterglow/Events/EventDispatcher.h"
#include "Afterglow/Events/WindowEvents.h"
#include "Afterglow/Events/MouseEvents.h"

namespace Afterglow
{
	Application::Application()
	{
		m_Window = std::unique_ptr<Window>(Window::Create());

		Subscribe<Event>(agBIND_FN(Application::OnEvent));
	}

	Application::~Application()
	{
	}
	
	void Application::Run()
	{
		while (m_Running)
		{
			for (Layer* layer : m_LayerStack)
				layer->OnUpdate();
				
			m_Window->OnUpdate();
		}
	}

	void Application::OnEvent(Event& e)
	{
		EventDispatcher disp(e);

		disp.Dispatch<WindowCloseEvent>(agBIND_FN(Application::OnWindowClose));
		disp.Dispatch<MouseMovedEvent>(agBIND_FN(Application::OnMouseMoved));

		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();)
		{
			(*--it)->OnEvent(e);
			if (e.IsHandled())
				break;
		}
	}

	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
	}

	void Application::PushOverlay(Layer* layer)
	{
		m_LayerStack.PushOverlay(layer);
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;
		AG_LOG_TRACE(e.ToString());

		return true;
	}

	bool Application::OnMouseMoved(MouseMovedEvent& e)
	{
		AG_LOG_TRACE(e.ToString());

		return true;
	}
}