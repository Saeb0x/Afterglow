#pragma once

#include "RendererAPI.h"

namespace Afterglow
{
	class RenderCommand
	{
	public:
		static void SetClearColor(const glm::vec4& color)
		{
			s_RendererAPI->SetClearColor(color);
		}

		static void Clear()
		{
			s_RendererAPI->Clear();
		}

		static void SetViewport(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
		{
			s_RendererAPI->SetViewport(x, y, width, height);
		}

		static void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray)
		{
			s_RendererAPI->DrawIndexed(vertexArray);
		}
	private:
		static RendererAPI* s_RendererAPI;
	};
}