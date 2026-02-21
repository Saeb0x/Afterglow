#include "Window.h"

#ifdef AG_PLATFORM_WINDOWS
	#include "Platform/Windows/WindowsWindow.h"
#endif

namespace Afterglow
{
	std::unique_ptr<Window> Window::Create(const WindowConfig& config)
	{
#ifdef AG_PLATFORM_WINDOWS
		return std::make_unique<WindowsWindow>(config);
#else
#error "Afterglow doesn't support this platform yet!"
		return nullptr;
#endif
	}
}