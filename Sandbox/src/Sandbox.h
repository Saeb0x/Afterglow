#pragma once

#include <Afterglow.h>

class Sandbox final : public Afterglow::Application
{
public:
	Sandbox()
		: Afterglow::Application("Sandbox")
	{
	}

	~Sandbox() override
	{
	}
};