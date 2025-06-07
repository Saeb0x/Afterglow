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
		Application();
		virtual ~Application();

		virtual void Run();
		
		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

	private:
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnMouseMoved(MouseMovedEvent& e);

	private:
		bool m_Running = true;
		std::unique_ptr<Window> m_Window;
		LayerStack m_LayerStack;
	};
}
