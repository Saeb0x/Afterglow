#include "agpch.h"
#include "WindowsWindow.h"

#include "Afterglow/Core/Base.h"

#include "Afterglow/Events/EventBus.h"
#include "Afterglow/Events/WindowEvents.h"
#include "Afterglow/Events/MouseEvents.h"
#include "Afterglow/Events/KeyboardEvents.h"

#include <glad/glad.h>

namespace Afterglow
{
	static bool s_GLFWInitialized = false;

	Window* Window::Create(const WindowProps& props)
	{
#if AG_PLATFORM_WINDOWS
		return new WindowsWindow(props);
#else
		AG_ASSERT(false, "Afterglow only supports Windows for now.");
		return nullptr;
#endif
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

		AG_LOG_INFO("Creating Window \"{0}\" ({1}, {2}).", m_Data.Title, m_Data.Width, m_Data.Height);

		if (!s_GLFWInitialized)
		{
			int initSuccess = glfwInit();
			AG_ASSERT(initSuccess, "Failed to initialize GLFW!");

			s_GLFWInitialized = true;
		}

		m_Window = glfwCreateWindow((int)m_Data.Width, (int)m_Data.Height, m_Data.Title.c_str(), nullptr, nullptr);
		glfwMakeContextCurrent(m_Window);
		glfwSwapInterval(props.VSync);
		SetVSync(props.VSync);

		int gladInit = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		AG_ASSERT(gladInit, "Failed to initialize GLAD!");

		glfwSetWindowUserPointer(m_Window, &m_Data);

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

		glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
			{
				switch (action)
				{
					case GLFW_RELEASE:
					{
						KeyReleasedEvent KREvent((const int)key);
						EventBus::GetInstance().Publish(KREvent);
						break;
					}
					case GLFW_PRESS:
					{
						KeyPressedEvent KPEvent((const int)key, false);
						EventBus::GetInstance().Publish(KPEvent);
						break;
					}
					case GLFW_REPEAT:
					{
						KeyPressedEvent KPREvent((const int)key, true);
						EventBus::GetInstance().Publish(KPREvent);
						break;
					}
					default:
					break;
				}
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