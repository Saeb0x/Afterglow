#include <stdio.h>
#include <Afterglow.h>

class Sandbox : public Afterglow::Application
{
public:
	Sandbox() = default;
	~Sandbox() override = default;

	void OnInit() override
	{
		AG_LOG_INFO("Sandbox initialized");

		int health = 100;
		float x = 10.5f;
		float y = 20.3f;

		AG_LOG_DEBUG("Simple message without formatting");
		AG_LOG_INFO("Player health: {}", health);
		AG_LOG_INFO("Position: ({}, {})", x, y);
		AG_LOG_WARN("Warning: health below {}, current: {}", 50, health);
		AG_LOG_ERROR("Error code: {}, message: {}", 404, "Not Found");
	}

	void OnUpdate() override 
	{
		AG_LOG_INFO("Updating Sandbox");
	}

	void OnShutdown() override
	{
		AG_LOG_INFO("Sandbox shutdown");
	}

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