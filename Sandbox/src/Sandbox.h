#pragma once

#include <Afterglow.h>
#include <imgui.h>

class DebugLayer : public Afterglow::Layer
{
public:
	DebugLayer() :
		Afterglow::Layer("Debug"), m_OrthographicCamera(-1.0f, 1.0f, -1.0f, 1.0f)
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
		if (Afterglow::Input::IsKeyPressed(AG_KEY_A))
			m_CameraPosition.x -= m_CameraMovementSpeed * ts;
		else if(Afterglow::Input::IsKeyPressed(AG_KEY_D))
			m_CameraPosition.x += m_CameraMovementSpeed * ts;

		if (Afterglow::Input::IsKeyPressed(AG_KEY_W))
			m_CameraPosition.y += m_CameraMovementSpeed * ts;
		else if (Afterglow::Input::IsKeyPressed(AG_KEY_S))
			m_CameraPosition.y -= m_CameraMovementSpeed * ts;

		if (Afterglow::Input::IsKeyPressed(AG_KEY_R))
			m_CameraRotation += m_CameraRotationSpeed * ts;

		m_OrthographicCamera.SetPosition(m_CameraPosition);
		m_OrthographicCamera.SetRotation(m_CameraRotation);

		Afterglow::Renderer::BeginScene(m_OrthographicCamera);
		Afterglow::Renderer::Submit(m_Shader, m_VertexArray);
		Afterglow::Renderer::EndScene();
	}

	void OnImGuiRender() override
	{
		static bool show = true;
		ImGui::ShowDemoWindow(&show);

		ImGui::Begin("Pic", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
		if (ImGui::ImageButton("pic", (ImTextureID)(intptr_t)m_Texture->GetRendererID(), ImVec2(128.0f, 128.0f)))
		{
			AG_APP_TRACE("You clicked on the pic!");
		}
		ImGui::End();
	}

private:
	std::shared_ptr<Afterglow::Texture2D> m_Texture;
	std::shared_ptr<Afterglow::Shader> m_Shader;
	std::shared_ptr<Afterglow::VertexArray> m_VertexArray;
	std::shared_ptr<Afterglow::VertexBuffer> m_VertexBuffer;
	std::shared_ptr<Afterglow::IndexBuffer> m_IndexBuffer;

	Afterglow::OrthographicCamera m_OrthographicCamera;
	glm::vec3 m_CameraPosition = { 0.0f, 0.0f, 0.0f };
	float m_CameraRotation = 0.0f;
	float m_CameraMovementSpeed = 1.0f;
	float m_CameraRotationSpeed = 45.0f;
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