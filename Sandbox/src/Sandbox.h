#pragma once

#include <Afterglow.h>

class Sandbox final : public Afterglow::Application
{
public:
	Sandbox()
	{
		PushOverlay(new Afterglow::ImGuiLayer());
	}
	~Sandbox() {}
};