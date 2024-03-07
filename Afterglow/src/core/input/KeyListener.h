#pragma once

#include <GLFW/glfw3.h>
#include <array>

namespace Afterglow {
	namespace Core {
		namespace Input {
			class KeyListener
			{
			public:
				static KeyListener& GetInstance();
				static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

				static bool IsKeyPressed(int key);
			private:
				KeyListener();
				~KeyListener();
				KeyListener(const KeyListener&);
				const KeyListener& operator=(const KeyListener&);
			private:
				std::array<bool, 350> m_KeysPressed;
			};
		}
	}
}

