#include "LevelEditorScene.h"

#include <iostream>

namespace Afterglow
{
	namespace Core
	{
		LevelEditorScene::LevelEditorScene() : Scene() {}
		LevelEditorScene::~LevelEditorScene() {}

		void LevelEditorScene::Init() 
		{
			std::cout << "I'm in the level editor Scene!" << std::endl;
		}
		void LevelEditorScene::Update(float deltaTime) {}
	}
}