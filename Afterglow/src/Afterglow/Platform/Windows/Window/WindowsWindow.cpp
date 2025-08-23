#include "agpch.h"
#include "WindowsWindow.h"

#include "Afterglow/Core/Base.h"
#include "Afterglow/Core/Events/WindowEvents.h"
#include "Afterglow/Core/Events/InputEvents.h"
#include "Afterglow/Platform/OpenGL/OpenGLContext.h"

#include <GLFW/glfw3.h>

namespace Afterglow
{
	static bool s_GLFWInitialized = false;

	Window* Window::Create(const WindowProps& props)
	{
#ifdef AG_PLATFORM_WINDOWS
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

		AG_INFO("Creating Window \"{0}\" ({1}, {2})", m_Data.Title, m_Data.Width, m_Data.Height);

		if (!s_GLFWInitialized)
		{
			int glfwInitialize = glfwInit();
			AG_ASSERT(glfwInitialize, "Failed to initialize GLFW!");

			s_GLFWInitialized = true;
		}

		m_Window = glfwCreateWindow((int)m_Data.Width, (int)m_Data.Height, m_Data.Title.c_str(), nullptr, nullptr);
	
		m_Context = new OpenGLContext(m_Window);
		m_Context->Init();

		SetVSync(props.VSync);

		glfwSetWindowUserPointer(m_Window, &m_Data);

		// GLFW Callbacks.
		glfwSetErrorCallback([](int error_code, const char* description)
			{
				AG_ERROR("GLFW error {0}: {1}", error_code, description);
			}
		);

		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				
				WindowCloseEvent WCEvent;
				data.EventCallback(WCEvent);
			}
		);

		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				data.Width = width;
				data.Height = height;

				WindowResizeEvent WREvent((unsigned int)width, (unsigned int)height);
				data.EventCallback(WREvent);
			}
		);

		glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xpos, double ypos)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

				MouseMovedEvent MMEvent((float)xpos, (float)ypos);
				data.EventCallback(MMEvent);
			}
		);

		glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

				switch (action)
				{
					case GLFW_RELEASE:
					{
						KeyReleasedEvent KREvent(key);
						data.EventCallback(KREvent);
						break;
					}
					case GLFW_PRESS:
					{
						KeyPressedEvent KPEvent(key, false);
						data.EventCallback(KPEvent);
						break;
					}
					case GLFW_REPEAT:
					{
						KeyPressedEvent KPREvent(key, true);
						data.EventCallback(KPREvent);
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
		delete m_Context;
		glfwDestroyWindow(m_Window);
	}

	void WindowsWindow::OnUpdate()
	{
		glfwPollEvents();
		m_Context->SwapBuffers();
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