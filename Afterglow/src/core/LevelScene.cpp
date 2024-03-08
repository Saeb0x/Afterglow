#include "LevelScene.h"
#include "Window.h"

#include <iostream>
#include <GLFW/glfw3.h>
#include "input/KeyListener.h"

namespace Afterglow
{
	namespace Core
	{
		LevelScene::LevelScene() : Scene(), m_SwitchScene(false), m_SwitchSceneTime(3.0f)
		{
			std::cout << "I'm in Level Scene!" << std::endl;
		}
		LevelScene::~LevelScene() {}

		void LevelScene::Update(float deltaTime)
		{
			if (!m_SwitchScene && Input::KeyListener::IsKeyPressed(GLFW_KEY_SPACE))
			{
				m_SwitchScene = true;
			}

			if (m_SwitchScene && m_SwitchSceneTime > 0)
			{
				m_SwitchSceneTime -= deltaTime;
			}
			else if (m_SwitchScene)
			{
				Window::ChangeScene(1);
			}
		}
	}
}