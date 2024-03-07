#include "KeyListener.h"

namespace Afterglow {
	namespace Core {
		namespace Input {
			KeyListener& KeyListener::GetInstance()
			{
				static KeyListener keyListenerInstance;
				return keyListenerInstance;
			}

			KeyListener::KeyListener()
			{
				this->m_KeysPressed = { false };
			}

			KeyListener::~KeyListener() {}

			void KeyListener::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
			{
				if (action == GLFW_PRESS)
				{
					if (key < GetInstance().m_KeysPressed.size())
					{
						GetInstance().m_KeysPressed[key] = true;
					}
				}
				else  if (action == GLFW_RELEASE)
				{
					if (key < GetInstance().m_KeysPressed.size())
					{
						GetInstance().m_KeysPressed[key] = false;
					}
				}
			}

			bool KeyListener::IsKeyPressed(int key)
			{
				if (key < GetInstance().m_KeysPressed.size())
				{
					return GetInstance().m_KeysPressed[key];
				}
				else
					return false;
			}

		}
	}
}
