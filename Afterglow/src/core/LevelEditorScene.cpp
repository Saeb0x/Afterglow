#include "LevelEditorScene.h"

#include <iostream>

namespace Afterglow
{
	namespace Core
	{
		LevelEditorScene::LevelEditorScene() : Scene() 
		{
			std::cout << "I'm in Level Editor Scene!" << std::endl;
		}
		LevelEditorScene::~LevelEditorScene() {}

		void LevelEditorScene::Update(float deltaTime)
		{

		}
	}
}