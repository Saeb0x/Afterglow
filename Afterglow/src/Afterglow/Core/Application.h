#pragma once

#include <memory>

#include "Window.h"
#include "LayerStack.h"
#include "Afterglow/Events/IEventSubscriber.h"

namespace Afterglow
{
	class WindowCloseEvent;
	class MouseMovedEvent;

	class Application : public IEventSubscriber
	{
	public:
		Application(const WindowProps& windowProps);
		virtual ~Application();
		Application(const Application&) = delete;
		Application& operator=(const Application&) = delete;
		Application(const Application&&) = delete;
		Application& operator=(Application&&) = delete;

		virtual void Run();
		
		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		static Application& Get() { return *s_Instance; }
		inline Window& GetWindow() const { return *m_Window; }

	private:
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnMouseMoved(MouseMovedEvent& e);

	protected:
		bool m_Running = true;
		std::unique_ptr<Window> m_Window;
		LayerStack m_LayerStack;

	private:
		static Application* s_Instance;
	};
}
