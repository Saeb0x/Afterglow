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
		AG_LOG_INFO("Updating Sandbox...");
	}
	void OnShutdown() override {}

	Afterglow::LoggerConfig GetLoggerConfig() override
	{
		Afterglow::LoggerConfig config;
		config.ClientLoggerName = "SANDBOX";
		config.ClientLogFileName = "Afterglow-Sandbox.log";

		return config;
	}
};

Afterglow::Application* Afterglow::CreateApplication()
{
	return new Sandbox();
}