#pragma once

#include "Core/GraphicsContext.h"
#include <Windows.h>

namespace Afterglow
{
	class OpenGLContext : public GraphicsContext
	{
	public:
		OpenGLContext(HWND windowHandle);
		~OpenGLContext() override;

		void Init() override;
		void SwapBuffers() override;
		void SetVSync(bool enabled) override;

	private:
		HWND m_WindowHandle;
		HDC m_DeviceContext;
		HGLRC m_RenderingContext;
	};
}