#include "agpch.h"
#include "WindowsWindow.h"

#include "Afterglow/Events/WindowEvents.h"
#include "Afterglow/Events/MouseEvents.h"
#include "Afterglow/Events/KeyEvents.h"

namespace Afterglow
{
	static bool m_GLFWInitialized = false;

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
		m_Data.m_Width = props.m_Width;
		m_Data.m_Height = props.m_Height;
		m_Data.m_Title = props.m_Title;
		m_Data.m_FullScreen = props.m_FullScreen;
		m_Data.m_VSync = props.m_VSync;

		AG_INFO("Creating window \"{0}\" ({1},{2})", m_Data.m_Title, m_Data.m_Width, m_Data.m_Height);

		if(!m_GLFWInitialized)
		{
			int success = glfwInit();
			AG_ASSERT(success, "Failed to initialize GLFW!");

			m_GLFWInitialized = true;
		}

		m_Window = glfwCreateWindow((int)m_Data.m_Width, (int)m_Data.m_Height, m_Data.m_Title.c_str(), nullptr, nullptr);
		glfwMakeContextCurrent(m_Window);
		glfwSetWindowUserPointer(m_Window, &m_Data);
		SetVSync(true);

		// Register Callbacks
		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
			{
				WindowCloseEvent event;

				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				data.m_EventCallback(event);
			}
		);

		glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xpos, double ypos)
			{
				MouseMoveEvent event((float)xpos, (float)ypos);

				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				data.m_EventCallback(event);
			}
		);
	}

	void WindowsWindow::OnUpdate()
	{
		glClear(GL_COLOR_BUFFER_BIT);
		glfwSwapBuffers(m_Window);
		glfwPollEvents();
	}

	unsigned int WindowsWindow::GetWidth() const
	{
		return m_Data.m_Width;
	}

	unsigned int WindowsWindow::GetHeight() const
	{
		return m_Data.m_Height;
	}

	void WindowsWindow::SetEventCallback(const EventCallbackFunc& callback)
	{
		m_Data.m_EventCallback = callback;
	}

	void WindowsWindow::SetVSync(bool enabled)
	{
		if (enabled)
			glfwSwapInterval(1);
		else
			glfwSwapInterval(0);

		m_Data.m_VSync = enabled;
	}

	bool WindowsWindow::IsVSync() const
	{
		return m_Data.m_VSync;
	}

	void WindowsWindow::Shutdown()
	{
		glfwDestroyWindow(m_Window);
		glfwTerminate();
	}
}
