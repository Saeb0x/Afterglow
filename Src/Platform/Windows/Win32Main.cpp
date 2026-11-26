#include "Core/Log.cpp"
#include "RHI/OpenGL/GLRHI.cpp"
#include "Win32Platform.cpp"
#include "App/App.cpp"

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR commandLine, int showCommand)
{
	(void)instance, (void)prevInstance, (void)commandLine, (void)showCommand;

	PlatformInitConsole();
	PlatformCreateWindow("Afterglow", 1280, 720);
	PlatformLoop();

	return (0);
}