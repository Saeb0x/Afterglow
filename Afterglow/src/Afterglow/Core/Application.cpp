#include "agpch.h"
#include "Application.h"

#include "Base.h"
#include "PlatformUtils.h"
#include "Timestep.h"

#include "Events/WindowEvents.h"
#include "Events/InputEvents.h"

#include "Renderer/Renderer.h"

namespace Afterglow
{
	Application::Application(const std::string& appName)
	{
		AG_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		m_Window = std::unique_ptr<Window>(Window::Create(WindowProps(appName)));
		m_Window->SetEventCallback(AG_BIND_FUNC(Application::OnEvent));

		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);

		m_VertexArray.reset(VertexArray::Create());

		float vertices[3 * 6] =
		{
			// a_vertexPos			// a_vertexColor
			-0.5f, -0.5f, 0.0f,		1.0f, 0.0f, 0.0f,
			0.0f,  0.5f,  0.0f,		0.0f, 1.0f, 0.0f,
			0.5f,  -0.5f, 0.0f,		0.0f, 0.0f, 1.0f
		};

		m_VertexBuffer.reset(VertexBuffer::Create(sizeof(vertices), vertices));

		{
			BufferLayout layout =
			{
				{ ShaderDataType::Float3, "a_vertexPos" },
				{ ShaderDataType::Float3, "a_vertexColor" }
			};

			m_VertexBuffer->SetLayout(layout);
		}
		
		m_VertexArray->AddVertexBuffer(m_VertexBuffer);

		uint32_t indices[3] =
		{
			0, 1, 2
		};

		m_IndexBuffer.reset(IndexBuffer::Create(3, indices));
		m_VertexArray->SetIndexBuffer(m_IndexBuffer);

		std::string vertexSource = R"(
			#version 330 core
			
			layout(location = 0) in vec3 a_vertexPos;
			layout(location = 1) in vec3 a_vertexColor;

			out vec3 v_vertexColor;
			
			void main()
			{
				gl_Position = vec4(a_vertexPos, 1.0);
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

		m_Shader.reset(Shader::Create(vertexSource, fragmentSource));
	}

	Application::~Application()
	{
		s_Instance = nullptr;
	}
	
	void Application::Run()
	{
		RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });

		while (b_Running)
		{
			float time = Time::GetTime();
			Timestep ts = time - m_LastFrameTime;
			m_LastFrameTime = time;

			RenderCommand::Clear();

			Renderer::BeginScene();

			m_Shader->Bind();
			Renderer::Submit(m_VertexArray);

			Renderer::EndScene();

			{
				for (Layer* layer : m_LayerStack)
					layer->OnUpdate(ts);
			}
			
			m_ImGuiLayer->Begin();
			{
				for (Layer* layer : m_LayerStack)
					layer->OnImGuiRender();
			}
			m_ImGuiLayer->End();

			m_Window->OnUpdate();
		}
	}

	void Application::Close()
	{
		b_Running = false;
	}

	void Application::OnEvent(Event& e)
	{
		EventDispatcher disp(e);

		disp.Dispatch<WindowCloseEvent>([this](Event& e)
			{
				b_Running = false;
				AG_WARNING(e.ToString());

				return true;
			}
		);

		for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
		{
			if (e.b_Handled)
				break;

			(*it)->OnEvent(e);
		}
	}

	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
	}

	void Application::PushOverlay(Layer* layer)
	{
		m_LayerStack.PushOverlay(layer);
	}
}