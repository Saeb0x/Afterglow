#include "TestScene.h"

#include <iostream>

namespace Afterglow
{
	namespace Core
	{
		using namespace Graphics;

		Renderer& renderer = Renderer::GetInstance();

		TestScene::TestScene() : Scene() {}

		TestScene::~TestScene()
		{
			delete vbo;
			delete layout;
			delete vao;
			delete ibo;
			delete shader;
		}

		void TestScene::Init()
		{
			std::cout << "I'm in the testing Scene!" << std::endl;

			float verticesData[8] =
			{
				// Position	 
				-0.5f, -0.5f,
				 0.5f, -0.5f,
				 0.5f,  0.5f,
				-0.5f,  0.5f
			};

			unsigned int indices[6] =
			{
				0, 1, 2,
				2, 3, 0
			};

			vao = new VertexArray();

		    vbo = new VertexBuffer(verticesData, 8 * sizeof(float));
			
			layout = new VertexLayout();
			layout->Push<float>(2);

			vao->AddBuffer(*vbo, *layout);

			vbo->Unbind();
			vao->Unbind();

			ibo = new IndexBuffer(indices, 6);
			ibo->Unbind();

			shader = new Shader("res/shaders/Vertex.glsl", "res/shaders/Fragment.glsl");
			shader->Bind();
			shader->SetUniform4f("u_FColor", 1.0f, 1.0f, 0.0f, 1.0f);

			shader->Unbind();
		}

		void TestScene::Update(float deltaTime)
		{
			renderer.Draw(*vao, *ibo, *shader);
		}
	}
}