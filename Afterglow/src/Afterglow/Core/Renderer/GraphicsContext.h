#pragma once

namespace Afterglow
{
	class GraphicsContext
	{
	public:
		GraphicsContext(void* windowHandle);
		virtual ~GraphicsContext();

		virtual void Init() = 0;
		virtual void SwapBuffers() = 0;

	protected:
		void* m_WindowHandle;
	};
}