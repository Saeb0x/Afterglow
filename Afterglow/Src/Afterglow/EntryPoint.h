#pragma once

#ifdef AG_PLATFORMS_WINDOWS

extern Afterglow::Application* Afterglow::CreateApplication();

int main(int argc, char** argv)
{
	std::cout << "Afterglow Engine\n";
	auto app = Afterglow::CreateApplication();
	app->Run();
	delete app;
}

#endif
