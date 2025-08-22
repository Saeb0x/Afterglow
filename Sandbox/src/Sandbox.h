#pragma once

#include <Afterglow.h>

#include <imgui.h>

class DebugLayer : public Afterglow::Layer
{
public:
	DebugLayer() :
		Afterglow::Layer("Debug")
	{
	}

	~DebugLayer() override = default;

	void OnAttach() override
	{
		AG_APP_TRACE("Layer \"{0}\" is attached", m_DebugName);
	}

	void OnImGuiRender() override
	{
		static bool show = true;
		ImGui::ShowDemoWindow(&show);
	}
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