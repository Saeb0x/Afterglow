#include <Afterglow.h>

class Sandbox : public Afterglow::Application
{
public:
	Sandbox()
	{
	}

	~Sandbox()
	{
	}
};

Afterglow::Application* Afterglow::CreateApplication()
{
	return new Sandbox;
}
