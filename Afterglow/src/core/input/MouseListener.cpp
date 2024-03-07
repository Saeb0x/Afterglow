#include "MouseListener.h"

namespace Afterglow {
	namespace Core {
		namespace Input {
			MouseListener& MouseListener::GetInstance()
			{
				static MouseListener mouseListenerInstance;
				return mouseListenerInstance;
			}

			MouseListener::MouseListener()
			{
				this->m_XPos = 0.0;
				this->m_YPos = 0.0;
				this->m_LastX = 0.0;
				this->m_LastY = 0.0;
				this->m_ScrollX = 0.0;
				this->m_ScrollY = 0.0;
				this->m_IsDragging = false;
				this->m_MouseButtonsPressed = { false };
			}

			MouseListener::~MouseListener() {}

			void MouseListener::CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
			{
				GetInstance().m_LastX = GetInstance().m_XPos;
				GetInstance().m_LastY = GetInstance().m_YPos;
				GetInstance().m_XPos = xpos;
				GetInstance().m_YPos = ypos;
				GetInstance().m_IsDragging = GetInstance().m_MouseButtonsPressed[0] || GetInstance().m_MouseButtonsPressed[1] || GetInstance().m_MouseButtonsPressed[2];
			}

			void MouseListener::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
			{
				if (action == GLFW_PRESS)
				{
					if (button < GetInstance().m_MouseButtonsPressed.size())
					{
						GetInstance().m_MouseButtonsPressed[button] = true;
					}
				}
				else if (action == GLFW_RELEASE)
				{
					if (button < GetInstance().m_MouseButtonsPressed.size())
					{
						GetInstance().m_MouseButtonsPressed[button] = false;
						GetInstance().m_IsDragging = false;
					}
				}
			}

			void MouseListener::MouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
			{
				GetInstance().m_ScrollX = xoffset;
				GetInstance().m_ScrollY = yoffset;
			}

			void MouseListener::EndFrame()
			{
				GetInstance().m_ScrollX = 0.0;
				GetInstance().m_ScrollY = 0.0;
				GetInstance().m_LastX = GetInstance().m_XPos;
				GetInstance().m_LastY = GetInstance().m_YPos;
			}

			float MouseListener::GetX()
			{
				return (float)GetInstance().m_XPos;
			}

			float MouseListener::GetY()
			{
				return (float)GetInstance().m_YPos;
			}

			float MouseListener::GetDx()
			{
				return (float)(GetInstance().m_LastX - GetInstance().m_XPos);
			}

			float MouseListener::GetDy()
			{
				return (float)(GetInstance().m_LastY - GetInstance().m_YPos);;
			}

			float MouseListener::GetScrollX()
			{
				return (float)GetInstance().m_ScrollX;
			}

			float MouseListener::GetScrollY()
			{
				return (float)GetInstance().m_ScrollY;
			}

			bool MouseListener::IsDragging()
			{
				return GetInstance().m_IsDragging;
			}

			bool MouseListener::IsMouseButtonPressed(int button)
			{
				if (button < GetInstance().m_MouseButtonsPressed.size())
				{
					return GetInstance().m_MouseButtonsPressed[button];
				}
				else
					return false;
			}
		}
	}
}
