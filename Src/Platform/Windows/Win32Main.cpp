#include "Core/Log.cpp"
#include "Win32Platform.cpp"

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR commandLine, int showCommand)
{
#if AG_DEBUG
	Win32ConsoleInit();
#endif

	AG_LOG_INFO("Afterglow started");

	return (0);
}