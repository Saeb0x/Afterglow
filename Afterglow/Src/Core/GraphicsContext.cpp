#include "GraphicsContext.h"

#ifdef AG_PLATFORM_WINDOWS
	#include "Platform/Graphics/OpenGL/OpenGLContext.h"
#endif

namespace Afterglow
{
	std::unique_ptr<GraphicsContext> GraphicsContext::Create(void* windowNativeHandle)
	{
#ifdef AG_PLATFORM_WINDOWS
		return std::make_unique<OpenGLContext>(static_cast<HWND>(windowNativeHandle));
#else
		#error "Afterglow doesn't support this platform yet!"
		return nullptr;
#endif
	}
}