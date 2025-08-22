#include "agpch.h"
#include "Application.h"

#include "Base.h"
#include "Events/WindowEvents.h"
#include "Events/InputEvents.h"

#include <glad/glad.h>

namespace Afterglow
{
	Application::Application(const std::string& appName)
	{
		AG_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		m_Window = std::unique_ptr<Window>(Window::Create(WindowProps(appName)));
		m_Window->SetEventCallback(AG_BIND_FUNC(Application::OnEvent));

		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);
	}

	Application::~Application()
	{
		s_Instance = nullptr;
	}
	
	void Application::Run()
	{
		glClearColor(0, 0, 0.3f, 1);

		while (b_Running)
		{
			glClear(GL_COLOR_BUFFER_BIT);

			{
				for (Layer* layer : m_LayerStack)
					layer->OnUpdate();
			}
			
			m_ImGuiLayer->Begin();
			{
				for (Layer* layer : m_LayerStack)
					layer->OnImGuiRender();
			}
			m_ImGuiLayer->End();

			m_Window->OnUpdate();
		}
	}

	void Application::Close()
	{
		b_Running = false;
	}

	void Application::OnEvent(Event& e)
	{
		EventDispatcher disp(e);

		disp.Dispatch<WindowCloseEvent>([this](Event& e)
			{
				b_Running = false;
				AG_WARNING(e.ToString());

				return true;
			}
		);

		for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
		{
			if (e.b_Handled)
				break;

			(*it)->OnEvent(e);
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
}