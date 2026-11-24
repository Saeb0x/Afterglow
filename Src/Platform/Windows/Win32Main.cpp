#include "Core/Log.cpp"
#include "Renderer/Renderer.cpp"
#include "Win32Platform.cpp"

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR commandLine, int showCommand)
{
	(void)instance, (void)prevInstance, (void)commandLine, (void)showCommand;

	PlatformInitConsole();
	PlatformCreateWindow("Afterglow", 1280, 720);
	PlatformLoop();

	return (0);
}