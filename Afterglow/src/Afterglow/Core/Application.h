#pragma once

#include "Window.h"
#include "Events/Event.h"

#include "LayerStack.h"
#include "Afterglow/ImGui/ImGuiLayer.h"

namespace Afterglow
{
	class Application
	{
	public:
		Application(const std::string& appName = "Afterglow App");
		virtual ~Application();

		virtual void Run();
		void Close();
		
		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		static Application& Get() { return *s_Instance; }
		Window& GetWindow() const { return *m_Window; }

		ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }

	private:
		inline static Application* s_Instance = nullptr;
		bool b_Running = true;
		float m_LastFrameTime = 0.0f;

		std::unique_ptr<Window> m_Window;
		bool b_Iconified = false;

		LayerStack m_LayerStack;
		ImGuiLayer* m_ImGuiLayer = nullptr;
	};
}