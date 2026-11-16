#include "Core/Log.cpp"
#include "Renderer/OpenGL/OpenGLRenderer.cpp"
#include "Win32Platform.cpp"

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR commandLine, int showCommand)
{
	PlatformInitConsole();
	PlatformCreateWindow("Afterglow", 1280, 720);

	PlatformGameLoop();

	PlatformCleanup();

	return (0);
}