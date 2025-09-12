#pragma once

#include <Afterglow.h>

#include <imgui.h>

class DebugLayer : public Afterglow::Layer
{
public:
	DebugLayer() :
		Afterglow::Layer("Debug"), m_OrthoCameraController(Afterglow::Application::Get().GetWindow().GetWidth(), Afterglow::Application::Get().GetWindow().GetHeight())
	{
		m_VertexArray.reset(Afterglow::VertexArray::Create());

		float vertices[3 * 6] =
		{
			// a_vertexPos			// a_vertexColor
			-0.5f, -0.5f, 0.0f,		1.0f, 0.0f, 0.0f,
			0.0f,  0.5f,  0.0f,		0.0f, 1.0f, 0.0f,
			0.5f,  -0.5f, 0.0f,		0.0f, 0.0f, 1.0f
		};

		m_VertexBuffer.reset(Afterglow::VertexBuffer::Create(sizeof(vertices), vertices));

		{
			Afterglow::BufferLayout layout =
			{
				{ Afterglow::ShaderDataType::Float3, "a_vertexPos" },
				{ Afterglow::ShaderDataType::Float3, "a_vertexColor" }
			};

			m_VertexBuffer->SetLayout(layout);
		}

		m_VertexArray->AddVertexBuffer(m_VertexBuffer);

		uint32_t indices[3] =
		{
			0, 1, 2
		};

		m_IndexBuffer.reset(Afterglow::IndexBuffer::Create(3, indices));
		m_VertexArray->SetIndexBuffer(m_IndexBuffer);

		std::string vertexSource = R"(
			#version 330 core
			
			layout(location = 0) in vec3 a_vertexPos;
			layout(location = 1) in vec3 a_vertexColor;

			uniform mat4 u_ProjectionViewMatrix;

			out vec3 v_vertexColor;
			
			void main()
			{
				gl_Position = u_ProjectionViewMatrix * vec4(a_vertexPos, 1.0);
				v_vertexColor = a_vertexColor;
			}
		)";

		std::string fragmentSource = R"(
			#version 330 core
			
			in vec3 v_vertexColor;
			out vec4 v_fragColor; 			

			void main()
			{
				v_fragColor = vec4(v_vertexColor, 1.0);
			}
		)";

		m_Shader.reset(Afterglow::Shader::Create(vertexSource, fragmentSource));

		m_Texture = Afterglow::Texture2D::Create("assets/textures/pic.jpeg");
	}

	~DebugLayer() override = default;

	void OnAttach() override
	{
		AG_APP_TRACE("Layer \"{0}\" is attached", m_DebugName);
	}

	void OnUpdate(Afterglow::Timestep ts) override
	{
		m_OrthoCameraController.OnUpdate(ts);

		Afterglow::Renderer::BeginScene(m_OrthoCameraController.GetCamera());
		Afterglow::Renderer::Submit(m_Shader, m_VertexArray);
		Afterglow::Renderer::EndScene();
	}

	void OnEvent(Afterglow::Event& event) override
	{
		m_OrthoCameraController.OnEvent(event);
	}

	void OnImGuiRender() override
	{
	}

private:
	std::shared_ptr<Afterglow::Texture2D> m_Texture;
	std::shared_ptr<Afterglow::Shader> m_Shader;
	std::shared_ptr<Afterglow::VertexArray> m_VertexArray;
	std::shared_ptr<Afterglow::VertexBuffer> m_VertexBuffer;
	std::shared_ptr<Afterglow::IndexBuffer> m_IndexBuffer;

	Afterglow::OrthographicCameraController m_OrthoCameraController;
};

class Sandbox final : public Afterglow::Application
{
public:
	Sandbox()
		: Afterglow::Application("Sandbox")
	{
		PushLayer(new DebugLayer());
	}

	~Sandbox() override
	{
	}
};