#pragma once

#include "../Scene.h"
#include <GLFW/glfw3.h>

namespace Afterglow
{
	namespace Core
	{
		namespace UI
		{
			class ImGuiLayer
			{
			public:
				static ImGuiLayer& GetInstance(GLFWwindow* mainWindow, bool enableDarkMode);

				void Update(float deltaTime, Scene& currentScene);
			private:
				ImGuiLayer(GLFWwindow* mainWindow, bool enableDarkMode);
				~ImGuiLayer();
				ImGuiLayer(const ImGuiLayer&);
				const ImGuiLayer& operator=(const ImGuiLayer&);
			private:
				GLFWwindow* m_ContextWindow;
				bool m_DarkMode;
			};
		}
	}
}

