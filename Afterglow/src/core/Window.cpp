#include "Window.h"
#include "input/MouseListener.h"
#include "input/KeyListener.h"

#include <iostream>

namespace Afterglow {
	namespace Core {
		Window& Window::GetInstance()
		{
			static Window winInstance;
			return winInstance;
		}

		Window::Window()
		{
			this->m_Width = 1920;
			this->m_Height = 1080;
			this->m_Title = "Afterglow";
			this->m_Window = nullptr;
		}

		Window::~Window() 
		{
			glfwDestroyWindow(m_Window);
			glfwTerminate();
		}

		void Window::Run()
		{
			Init();
			Loop();
		}

		void Window::Init()
		{
			// Initialize GLFW
			if (!glfwInit())
			{
				std::cerr << "Unable to initialize GLFW." << std::endl;
				return;
			}
			std::cout << "GLFW is initialized!" << std::endl;

			// Configure GLFW
			glfwDefaultWindowHints();
			glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
			glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
			glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

			// Create the window
			m_Window = glfwCreateWindow(m_Width, m_Height, m_Title, NULL, NULL);
			if (!m_Window)
			{
				std::cerr << "Unable to create the window." << std::endl;
				glfwTerminate();
				return;
			}
			std::cout << "Window is created!" << std::endl;

			// Make OpenGL context current
			glfwMakeContextCurrent(m_Window);

			// Enable V-Sync
			glfwSwapInterval(1);

			// Register Callbacks
			glfwSetCursorPosCallback(m_Window, Input::MouseListener::CursorPosCallback);
			glfwSetMouseButtonCallback(m_Window, Input::MouseListener::MouseButtonCallback);
			glfwSetScrollCallback(m_Window, Input::MouseListener::MouseScrollCallback);
			glfwSetKeyCallback(m_Window, Input::KeyListener::KeyCallback);

			// Make window visible
			glfwShowWindow(m_Window);
		}

		void Window::Loop()
		{
			while (!glfwWindowShouldClose(m_Window))
			{
				// Process all pending events
				glfwPollEvents();
				
				glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
				glClear(GL_COLOR_BUFFER_BIT);

				glfwSwapBuffers(m_Window);
			}
		}
	}
}
