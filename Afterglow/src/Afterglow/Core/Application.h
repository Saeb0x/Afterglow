#pragma once

#include "Window.h"
#include "Events/Event.h"

#include "LayerStack.h"
#include "ImGui/ImGuiLayer.h"

#include "Renderer/Shader.h"
#include "Renderer/Buffer.h"
#include "Renderer/VertexArray.h"

namespace Afterglow
{
	class Application
	{
	public:
		Application(const std::string& appName = "Afterglow App");
		virtual ~Application();
		Application(const Application&) = delete;
		Application& operator=(const Application&) = delete;
		Application(const Application&&) = delete;
		Application& operator=(Application&&) = delete;

		virtual void Run();
		void Close();
		
		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		static Application& Get() { return *s_Instance; }
		Window& GetWindow() const { return *m_Window; }

	private:
		inline static Application* s_Instance = nullptr;

		bool b_Running = true;
		std::unique_ptr<Window> m_Window;

		ImGuiLayer* m_ImGuiLayer = nullptr;
		LayerStack m_LayerStack;

		std::unique_ptr<Shader> m_Shader;
		std::shared_ptr<VertexArray> m_VertexArray;
		std::shared_ptr<VertexBuffer> m_VertexBuffer;
		std::shared_ptr<IndexBuffer> m_IndexBuffer;
	};
}
