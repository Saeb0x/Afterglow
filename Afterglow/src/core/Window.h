#pragma once

#include <GLFW/glfw3.h>

namespace Afterglow {
	namespace Core {
		class Window
		{
		public:
			static Window& GetInstance();
			void Run();
		private:
			Window();
			~Window();
			Window(const Window&);
			const Window& operator=(const Window&);
		private:
			int m_Width, m_Height;
			const char* m_Title;
			GLFWwindow* m_Window;
		private:
			void Init();
			void Loop();
		};
	}
}


