#include <iostream>
#include <cassert>

#include "Window.h"
#include "input/MouseListener.h"
#include "input/KeyListener.h"
#include "utils/Timer.h"
#include "TestScene.h"

namespace Afterglow 
{
	namespace Core 
	{
		Scene* Window::m_CurrentScene = nullptr;

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

		void Window::ChangeScene(int newScene)
		{
			delete m_CurrentScene;
			m_CurrentScene = nullptr;

			switch (newScene)
			{
			case 0:
				m_CurrentScene = new TestScene();
				m_CurrentScene->Init();
				break;
			default:
				assert(false && "Unresolved Scene Index");
				break;
			}
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
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

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

			if (glewInit() != GLEW_OK)
			{
				std::cerr << "Unable to initialize GLEW." << std::endl;
				glfwDestroyWindow(m_Window);
				glfwTerminate();
				return;
			}
			std::cout << glGetString(GL_VERSION) << std::endl;

			// Enable V-Sync
			glfwSwapInterval(1);
			
			// Enable Blending
			using namespace Graphics;
			GLCall(glEnable(GL_BLEND));
			GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
			
			// Register Callbacks
			glfwSetCursorPosCallback(m_Window, Input::MouseListener::CursorPosCallback);
			glfwSetMouseButtonCallback(m_Window, Input::MouseListener::MouseButtonCallback);
			glfwSetScrollCallback(m_Window, Input::MouseListener::MouseScrollCallback);
			glfwSetKeyCallback(m_Window, Input::KeyListener::KeyCallback);
			glfwSetFramebufferSizeCallback(m_Window, FramebufferSizeCallback);

			// Make window visible
			glfwShowWindow(m_Window);

			ChangeScene(0);
		}

		void Window::Loop()
		{
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			float Dt = -1.0f;

			float beginTime = Utils::Timer::GetTime();
			while (!glfwWindowShouldClose(m_Window))
			{
				// Process all pending events
				glfwPollEvents();

				glClear(GL_COLOR_BUFFER_BIT);

				if(Dt > 0)
					m_CurrentScene->Update(Dt);
				
				glfwSwapBuffers(m_Window); 

				float endTime = Utils::Timer::GetTime();
				Dt = endTime - beginTime;
				beginTime = endTime;
			}

			delete m_CurrentScene;
		}

		void Window::FramebufferSizeCallback(GLFWwindow* window, int width, int height)
		{
			glViewport(0, 0, width, height);
		}
	}
}
