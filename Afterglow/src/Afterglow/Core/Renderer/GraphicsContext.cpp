#include "agpch.h"
#include "GraphicsContext.h"

#include "Afterglow/Core/Assert.h"

namespace Afterglow
{
	GraphicsContext::GraphicsContext(void* windowHandle)
		: m_WindowHandle(windowHandle)
	{
		AG_ASSERT(windowHandle, "Window handle is null!");
	}

	GraphicsContext::~GraphicsContext()
	{
	}
}