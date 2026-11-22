#include "Core/Log.cpp"
#include "Rendering/RHI/OpenGL/OpenGLRHI.cpp"
#include "Win32Platform.cpp"
#include "App/App.cpp"

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR commandLine, int showCommand)
{
	PlatformInitConsole();
	PlatformCreateWindow("Afterglow", 1280, 720);

	PlatformGameLoop();

	return (0);
}