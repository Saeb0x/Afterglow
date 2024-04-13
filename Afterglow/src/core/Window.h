#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Scene.h"
#include "ui/ImGuiLayer.h"

namespace Afterglow {
	namespace Core {
		class Window
		{
		public:
			static Window& GetInstance();
			void Run();
			static void ChangeScene(int newScene);
			inline static Scene* GetScene() { return m_CurrentScene; }
			inline static int GetWidth() { return GetInstance().m_Width; }
			inline static int GetHeight() { return GetInstance().m_Height; }
		private:
			Window();
			~Window();
			Window(const Window&);
			const Window& operator=(const Window&);
		private:
			int m_Width, m_Height;
			const char* m_Title;
			GLFWwindow* m_Window;
			static Scene* m_CurrentScene;
			static UI::ImGuiLayer* m_ImGuiLayer;
		private:
			void Init();
			void Loop();
			static void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
		};
	}
}


