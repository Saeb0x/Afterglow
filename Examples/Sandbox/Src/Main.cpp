#include <stdio.h>
#include <Afterglow.h>

class Sandbox : public Afterglow::Application
{
public:
	Sandbox() = default;
	~Sandbox() override = default;

	Afterglow::LoggerConfig GetLoggerConfig() override
	{
		Afterglow::LoggerConfig config;
		config.ClientLoggerName = "SANDBOX";
		config.ClientLogFileName = "Afterglow-Sandbox.log";

		return config;
	}

	Afterglow::WindowConfig GetWindowConfig() override
	{
		Afterglow::WindowConfig config;
		config.Title = "Sandbox";
		config.Resizable = true;

		return config;
	}

	void OnInit() override 
	{
		AG_LOG_INFO("Window size: {}x{}", GetWindow().GetWidth(), GetWindow().GetHeight());
	}

	void OnUpdate() override 
	{
		if (Afterglow::Input::IsKeyPressed(Afterglow::Key::Escape))
		{
			AG_LOG_DEBUG("Escape pressed - closing application");
			Close();
		}

		if (Afterglow::Input::IsKeyDown(Afterglow::Key::W))
		{
			AG_LOG_DEBUG("W is down");
		}

		if (Afterglow::Input::IsKeyPressed(Afterglow::Key::Space))
		{
			AG_LOG_DEBUG("Space pressed!");
		}

		if (Afterglow::Input::IsMouseButtonPressed(Afterglow::MouseButton::Left))
		{
			auto [x, y] = Afterglow::Input::GetMousePosition();
			AG_LOG_DEBUG("Left mouse clicked at ({}, {})", x, y);
		}

		static int frameCount = 0;
		if (++frameCount % 60 == 0)
		{
			auto [x, y] = Afterglow::Input::GetMousePosition();
			// AG_LOG_DEBUG("Mouse position: ({}, {})", x, y);
		}
	}

	void OnShutdown() override {}
};

Afterglow::Application* Afterglow::CreateApplication()
{
	return new Sandbox();
}