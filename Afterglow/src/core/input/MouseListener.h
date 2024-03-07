#pragma once

#include <GLFW/glfw3.h>
#include <array>

namespace Afterglow {
	namespace Core {
		namespace Input {

			class MouseListener
			{
			public:
				static MouseListener& GetInstance();

				static void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);
				static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
				static void MouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

				static void EndFrame();
				static float GetX();
				static float GetY();
				static float GetDx();
				static float GetDy();
				static float GetScrollX();
				static float GetScrollY();
				static bool IsDragging();
				static bool IsMouseButtonPressed(int button);
			private:
				MouseListener();
				~MouseListener();
				MouseListener(const MouseListener&);
				const MouseListener& operator=(const MouseListener&);
			private:
				double m_XPos, m_YPos, m_LastX, m_LastY, m_ScrollX, m_ScrollY;
				bool m_IsDragging;
				std::array<bool, 3> m_MouseButtonsPressed;
			};
		}
	}
}
