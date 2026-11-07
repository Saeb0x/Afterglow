#include "Core/Utility.h"
#include "Core/Log.cpp"
#include "App/App.cpp"
#include "Win32Platform.cpp"

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR commandLine, int showCommand)
{
	Win32ConsoleInit();

	usize memorySize = AG_MIB(64);
	void* memory = PlatformAllocateMemory(memorySize);
	if(!memory)
	{
		PlatformFatalError("Couldn't allocate %zu bytes of memory.", memorySize);
	}

	WindowConfig config = AppGetWindowConfig();
	HWND window = Win32CreateWindow(instance, &config);

	Win32PumpMessages();

	DestroyWindow(window);
	PlatformFreeMemory(memory);

	return (0);
}