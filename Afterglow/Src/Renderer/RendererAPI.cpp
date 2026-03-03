#include "RendererAPI.h"

#ifdef AG_PLATFORM_WINDOWS
	#include "Platform/Graphics/OpenGL/OpenGLRendererAPI.h"
#endif

namespace Afterglow
{
	// NOTE(saeb): Default to OpenGL for now
	RendererAPI::API RendererAPI::s_API = RendererAPI::API::OpenGL;

	std::unique_ptr<RendererAPI> RendererAPI::Create()
	{
		switch (s_API)
		{
			case API::OpenGL:
			{
#ifdef AG_PLATFORM_WINDOWS
				return std::make_unique<OpenGLRendererAPI>();
#else
				#error "OpenGL not supported on this platform!"
#endif
			}
			default:
				return nullptr;
		}
	}
}