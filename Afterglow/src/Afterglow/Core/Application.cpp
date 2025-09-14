#include "agpch.h"
#include "Application.h"

#include "Base.h"
#include "PlatformUtils.h"
#include "Timestep.h"

#include "Events/WindowEvents.h"
#include "Events/InputEvents.h"

#include "Afterglow/Renderer/Renderer2D.h"

namespace Afterglow
{
	Application::Application(const std::string& appName)
	{
		AG_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		m_Window = std::unique_ptr<Window>(Window::Create(WindowProps(appName)));
		m_Window->SetEventCallback(AG_BIND_FUNC(Application::OnEvent));

		Renderer2D::GetInstance().Init();

		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);
	}

	Application::~Application()
	{
		s_Instance = nullptr;
	}
	
	void Application::Run()
	{
		Afterglow::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		while (b_Running)
		{
			float time = Time::GetTime();
			Timestep ts = time - m_LastFrameTime;
			m_LastFrameTime = time;
			
			Afterglow::RenderCommand::Clear();

			if (!b_Iconified)
			{
				{
					for (Layer* layer : m_LayerStack)
						layer->OnUpdate(ts);
				}

				m_ImGuiLayer->Begin();
				{
					for (Layer* layer : m_LayerStack)
						layer->OnImGuiRender();
				}
				m_ImGuiLayer->End();
			}
			
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

		disp.Dispatch<WindowCloseEvent>([this](WindowCloseEvent& e)
			{
				b_Running = false;
				AG_WARNING(e.ToString());

				return true;
			}
		);

		disp.Dispatch<WindowIconifyEvent>([this](WindowIconifyEvent& e)
			{
				b_Iconified = e.GetIconify();
				AG_WARNING(e.ToString());

				return true;
			}
		);

		disp.Dispatch<WindowResizeEvent>([this](WindowResizeEvent& e)
			{
				Renderer2D::GetInstance().SetViewport(0, 0, (uint16_t)e.GetWidth(), (uint16_t)e.GetHeight());
				AG_WARNING("Window viewport resized: {}, {}", (uint16_t)e.GetWidth(), (uint16_t)e.GetHeight());
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