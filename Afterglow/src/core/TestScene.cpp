#include "TestScene.h"

#include <iostream>

namespace Afterglow
{
	namespace Core
	{
		using namespace Graphics;

		float x, y = 0.0f;

		TestScene::TestScene() : Scene() {}

		TestScene::~TestScene()
		{
			delete vbo;
			delete layout;
			delete vao;
			delete ibo;
			delete shader;
			delete texture;

			delete m_OrthographicCamera;
		}

		void TestScene::Init()
		{
			std::cout << "I'm in the testing Scene!" << std::endl;

			float verticesData[4*7] =
			{
				 // Position	 // Color		    // UV
				 0.0f,   0.0f,	 1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
				 200.0f, 0.0f,	 0.0f, 1.0f, 0.0f,  1.0f, 0.0f, 
				 200.0f, 200.0f, 0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
				 0.0f,   200.0f, 1.0f, 1.0f, 0.0f,  0.0f, 1.0f
			};

			unsigned int indices[6] =
			{
				0, 1, 2,
				2, 3, 0
			};

			vao = new VertexArray();

		    vbo = new VertexBuffer(verticesData, 4 * 7 * sizeof(float));
			
			layout = new VertexLayout();
			layout->Push<float>(2);
			layout->Push<float>(3);
			layout->Push<float>(2);

			vao->AddBuffer(*vbo, *layout);

			vbo->Unbind();
			vao->Unbind();

			ibo = new IndexBuffer(indices, 6);
			ibo->Unbind();

			m_OrthographicCamera = new OrthographicCamera(0.0f, 32.0f * 40.0f, 0.0f, 32.0f * 21.0f);

			shader = new Shader("res/shaders/Vertex.glsl", "res/shaders/Fragment.glsl");
			shader->Bind();

			texture = new Texture("res/textures/logo.png");
			texture->Bind();
			shader->SetUniform1i("u_Texture", 0);
			shader->Unbind();
		}

		void TestScene::Update(float deltaTime)
		{
			m_Renderer.Draw(*vao, *ibo, *shader);

			x -= deltaTime * 50.0f;
			y -= deltaTime * 50.0f;
			m_OrthographicCamera->SetPosition({ x, y, 0.0f });
			shader->SetUniformMatrix4fv("u_ProjectionViewMatrix", m_OrthographicCamera->GetProjectionViewMatrix());
		}
	}
}