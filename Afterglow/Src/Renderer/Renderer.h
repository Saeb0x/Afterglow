#pragma once

#include "RendererAPI.h"
#include <memory>

namespace Afterglow
{
	class Renderer
	{
	public:
		static void Init();
		static void Shutdown();

		static RendererAPI& GetAPI() { return *s_RendererAPI; }

	private:
		static std::unique_ptr<RendererAPI> s_RendererAPI;
	};
}