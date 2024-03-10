#include "TestScene.h"

#include <GLFW/glfw3.h>
#include <iostream>

namespace Afterglow
{
	namespace Core
	{
		TestScene::TestScene() : Scene() {}
		TestScene::~TestScene() {}

		void TestScene::Init()
		{
			std::cout << "I'm in the testing Scene!" << std::endl;
 		}

		void TestScene::Update(float deltaTime)
		{
			glBegin(GL_TRIANGLES);
			glVertex2f(-0.5,-0.5);
			glVertex2f(0,0.5);
			glVertex2f(0.5,-0.5);
			glEnd();
		}
	}
}
