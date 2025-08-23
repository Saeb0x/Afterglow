#include "agpch.h"
#include "Application.h"

#include "Base.h"
#include "Events/WindowEvents.h"
#include "Events/InputEvents.h"

#include <glad/glad.h>

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

		glGenVertexArrays(1, &m_VertexArray);
		glBindVertexArray(m_VertexArray);

		glGenBuffers(1, &m_VertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);

		float vertices[3 * 3] =
		{
			-0.5f, -0.5f, 0.0f,
			0.0f, 0.5f, 0.0f,
			0.5f, -0.5f, 0.0f 
		};

		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);

		glGenBuffers(1, &m_IndexBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexBuffer);

		uint32_t indices[3] =
		{
			0, 1, 2
		};

		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		std::string vertexSource = R"(
			#version 460 core
			
			layout(location = 0) in vec3 a_vertexPos; 			

			out vec3 vertexPos;
			
			void main()
			{
				vertexPos = a_vertexPos;
				gl_Position = vec4(a_vertexPos, 1.0);
			}
		)";

		std::string fragmentSource = R"(
			#version 460 core
			
			in vec3 vertexPos;
			out vec4 fragColor; 			

			void main()
			{
				fragColor = vec4(vertexPos * 0.5 + 0.5, 1.0);
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
		glClearColor(0.1f, 0.1f, 0.1f, 1);

		while (b_Running)
		{
			glClear(GL_COLOR_BUFFER_BIT);

			m_Shader->Bind();
			glBindVertexArray(m_VertexArray);
			glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, nullptr);

			{
				for (Layer* layer : m_LayerStack)
					layer->OnUpdate();
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