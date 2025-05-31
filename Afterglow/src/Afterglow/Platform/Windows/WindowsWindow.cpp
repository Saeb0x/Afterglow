#include "agpch.h"
#include "WindowsWindow.h"

#include "Afterglow/Core/Base.h"

#include "Afterglow/Events/EventBus.h"
#include "Afterglow/Events/WindowEvents.h"
#include "Afterglow/Events/MouseEvents.h"

namespace Afterglow
{
	static bool s_GLFWInitialized = false;

	Window* Window::Create(const WindowProps& props)
	{
		return new WindowsWindow(props);
	}

	WindowsWindow::WindowsWindow(const WindowProps& props)
	{
		Init(props);
	}

	WindowsWindow::~WindowsWindow()
	{
		Shutdown();
	}

	void WindowsWindow::Init(const WindowProps& props)
	{
		m_Data.Title = props.Title;
		m_Data.Width = props.Width;
		m_Data.Height = props.Height;
		m_Data.VSync = props.VSync;

		AG_LOG_INFO("Creating Window \"{0}\" ({1}, {2}).", m_Data.Title, m_Data.Width, m_Data.Height);

		if (!s_GLFWInitialized)
		{
			int initSuccess = glfwInit();
			AG_ASSERT(initSuccess, "Could not initialize GLFW.");

			s_GLFWInitialized = true;
		}

		m_Window = glfwCreateWindow((int)m_Data.Width, (int)m_Data.Height, m_Data.Title.c_str(), nullptr, nullptr);
		glfwMakeContextCurrent(m_Window);
		glfwSetWindowUserPointer(m_Window, &m_Data);
		glfwSwapInterval(m_Data.VSync);

		// GLFW Callbacks.
		glfwSetErrorCallback([](int error_code, const char* description)
			{
				AG_LOG_ERROR("GLFW error {0}: {1}", error_code, description);
			}
		);

		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
			{
				WindowCloseEvent e;
				EventBus::GetInstance().Publish(e);
			}
		);

		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
			{
				WindowProps& props = *(WindowProps*)glfwGetWindowUserPointer(window);
				props.Width = width;
				props.Height = height;

				WindowResizeEvent e((unsigned int)width, (unsigned int)height);
				EventBus::GetInstance().Publish(e);
			}
		);

		glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xpos, double ypos)
			{
				MouseMovedEvent e((float)xpos, (float)ypos);
				EventBus::GetInstance().Publish(e);
			}
		);
	}

	void WindowsWindow::Shutdown()
	{
		glfwDestroyWindow(m_Window);
	}

	void WindowsWindow::OnUpdate()
	{
		glfwPollEvents();
		glfwSwapBuffers(m_Window);
	}

	void WindowsWindow::SetVSync(bool enabled)
	{
		if (m_Data.VSync != enabled)
		{
			if (enabled)
				glfwSwapInterval(1);
			else
				glfwSwapInterval(0);

			m_Data.VSync = enabled;
		}
	}
}