#pragma once

#include "Afterglow/Core/Renderer/GraphicsContext.h"

namespace Afterglow
{
	class OpenGLContext : public GraphicsContext
	{
	public:
		OpenGLContext(void* windowHandle);
		~OpenGLContext() override = default;

		void Init() override;
		void SwapBuffers() override;
		void* GetWindowHandle() const override { return m_WindowHandle; }

	private:
		void* m_WindowHandle;
	};
}
