#pragma once

#include <Afterglow.h>

class DebugLayer : public Afterglow::Layer
{
public:
	DebugLayer() : Layer("DebugLayer")
	{
	}

	inline void OnUpdate() override
	{
		if (Afterglow::Input::IsKeyPressed(AG_KEY_T))
		{
			AG_LOG_TRACE("Key {0} is pressed!", (char)AG_KEY_T);
		}
	}
};

class Sandbox final : public Afterglow::Application
{
public:
	Sandbox()
		: Afterglow::Application(Afterglow::WindowProps())
	{
		PushLayer(new DebugLayer());
		PushOverlay(new Afterglow::ImGuiLayer());
	}

	~Sandbox() override
	{
	}
};