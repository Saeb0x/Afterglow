#include "agpch.h"
#include "RenderCommand.h"

#include "Afterglow/Platform/OpenGL/OpenGLRendererAPI.h"

namespace Afterglow
{
	RendererAPI* RenderCommand::s_RendererAPI = new OpenGLRendererAPI();
	
	void RenderCommand::SetClearColor(const glm::vec4& color)
	{
		s_RendererAPI->SetClearColor(color);
	}

	void RenderCommand::Clear()
	{
		s_RendererAPI->Clear();
	}

	void RenderCommand::SetViewport(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
	{
		s_RendererAPI->SetViewport(x, y, width, height);
	}

	void RenderCommand::BindTexture(uint32_t rendererID)
	{
		s_RendererAPI->BindTexture(rendererID);
	}

	void RenderCommand::DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray)
	{
		s_RendererAPI->DrawIndexed(vertexArray);
	}
}