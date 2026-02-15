#include <stdio.h>
#include <Afterglow.h>

class Sandbox : public Afterglow::Application
{
public:
	Sandbox() = default;
	~Sandbox() override = default;

	void OnInit() override {}
	void OnUpdate() override 
	{
		printf("Updating Sandbox!\n");
	}
	void OnShutdown() override {}
};

Afterglow::Application* Afterglow::CreateApplication()
{
	return new Sandbox();
}