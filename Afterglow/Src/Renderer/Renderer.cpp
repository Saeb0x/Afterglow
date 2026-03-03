#include "Renderer.h"
#include "Core/Log.h"

namespace Afterglow
{
	std::unique_ptr<RendererAPI> Renderer::s_RendererAPI = nullptr;

	void Renderer::Init()
	{
		s_RendererAPI = RendererAPI::Create();
		s_RendererAPI->Init();
	}

	void Renderer::Shutdown()
	{
		s_RendererAPI.reset();
	}
}