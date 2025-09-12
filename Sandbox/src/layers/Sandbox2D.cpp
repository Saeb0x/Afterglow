#include "Sandbox2D.h"

Sandbox2D::Sandbox2D()
	: Afterglow::Layer("Sandbox2D"), m_OrthoCameraController(Afterglow::Application::Get().GetWindow().GetWidth(), Afterglow::Application::Get().GetWindow().GetHeight())
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

void Sandbox2D::OnAttach()
{
	AG_APP_TRACE("Layer \"{0}\" is attached", m_DebugName);
}

void Sandbox2D::OnDetach()
{
	AG_APP_TRACE("Layer \"{0}\" is detached", m_DebugName);
}

void Sandbox2D::OnUpdate(Afterglow::Timestep ts)
{
	m_OrthoCameraController.OnUpdate(ts);

	Afterglow::Renderer::BeginScene(m_OrthoCameraController.GetCamera());
	Afterglow::Renderer::Submit(m_Shader, m_VertexArray);
	Afterglow::Renderer::EndScene();
}

void Sandbox2D::OnEvent(Afterglow::Event& event)
{
	m_OrthoCameraController.OnEvent(event);
}

void Sandbox2D::OnImGuiRender()
{
}

