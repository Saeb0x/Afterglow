#include "agpch.h"
#include "Application.h"

namespace Afterglow
{
	Application::Application()
	{
		m_Window = std::unique_ptr<Window>(Window::Create());
	}
	
	void Application::Run()
	{
		while (m_Running)
		{
			m_Window->OnUpdate();
		}
	}
}