#include "Core/Log.cpp"
#include "Renderer/OpenGL/OpenGLRenderer.cpp"
#include "Win32Platform.cpp"

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR commandLine, int showCommand)
{
	Win32ConsoleInit();

	HWND window = Win32CreateWindow(instance, "Afterglow", 1280, 720);

	Win32PumpMessages();

	DestroyWindow(window);

	return (0);
}