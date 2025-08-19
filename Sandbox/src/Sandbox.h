#pragma once

#include <Afterglow.h>
#include <imgui.h>

class DebugLayer : public Afterglow::Layer
{
public:
	DebugLayer()
		: Afterglow::Layer("Debug")
	{
	}
	~DebugLayer() override = default;

	void OnAttach() override
	{
		AG_CLIENT_LOG_INFO("Layer \"{0}\" is attached!", m_DebugName);
	}

	void OnEvent(Afterglow::Event& e) override
	{
		if (Afterglow::Input::IsKeyPressed(AG_KEY_SPACE))
		{
			AG_CLIENT_LOG_TRACE("Space key is clicked and consumed by the \"{0}\" layer!", m_DebugName);
		}
	}

	void OnImGuiRender() override
	{
		ImGui::ShowDemoWindow();
	}
};

class Sandbox final : public Afterglow::Application
{
public:
	Sandbox()
		: Afterglow::Application(Afterglow::WindowProps())
	{
		PushLayer(new DebugLayer());
	}

	~Sandbox() override
	{
	}
};