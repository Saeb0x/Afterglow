#pragma once

namespace Afterglow
{
	enum class RendererAPI
	{
		None = 0,
		OpenGL = 1
	};

	class Renderer
	{
	public:
		static RendererAPI GetAPI() { return s_RendererAPI; }

	private:
		inline static RendererAPI s_RendererAPI = RendererAPI::OpenGL;
	};
}

