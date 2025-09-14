#pragma once

namespace Afterglow
{
	class GraphicsContext
	{
	public:
		virtual ~GraphicsContext() = default;

		virtual void Init() = 0;
		virtual void SwapBuffers() = 0;
		virtual void* GetWindowHandle() const = 0;

		static std::shared_ptr<GraphicsContext> Create(void* windowHandle);
	};
}