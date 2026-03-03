#pragma once

#include <memory>

namespace Afterglow
{
	class RendererAPI
	{
	public:
		enum class API
		{
			OpenGL
		};

	public:
		virtual ~RendererAPI() = default;

		virtual void Init() = 0;
		virtual void SetClearColor(float r, float g, float b, float a) = 0;
		virtual void Clear() = 0;
		
		static API GetAPI() { return s_API; }
		static void SetAPI(API api) { s_API = api; }

		static std::unique_ptr<RendererAPI> Create();

	private:
		static API s_API;
	};
}