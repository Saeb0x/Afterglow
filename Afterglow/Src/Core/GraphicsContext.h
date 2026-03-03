#pragma once

#include <memory>

namespace Afterglow
{
	class GraphicsContext
	{
	public:
		virtual ~GraphicsContext() = default;

		virtual void Init() = 0;
		virtual void SwapBuffers() = 0;
		virtual void SetVSync(bool enabled) = 0;

		static std::unique_ptr<GraphicsContext> Create(void* windowNativeHandle);
	};
}