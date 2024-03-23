#include "TestScene.h"

#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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

			delete orthoCamera;
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

			orthoCamera = new OrthographicCamera(-1.6f, 1.6f, -0.9f, 0.9f);

			shader = new Shader("res/shaders/Vertex.glsl", "res/shaders/Fragment.glsl");
			shader->Bind();
			shader->SetUniform4f("u_FColor", 1.0f, 0.0f, 0.0f, 1.0f);
			shader->SetUniformMatrix4fv("u_ProjectionViewMatrix", orthoCamera->GetProjectionViewMatrix());
			
			shader->Unbind();
		}

		void TestScene::Update(float deltaTime)
		{
			renderer.Draw(*vao, *ibo, *shader);
		}
	}
}