#pragma once

#include "RendererAPI.h"

namespace Afterglow
{
	class RenderCommand
	{
	public:
		static void SetClearColor(const glm::vec4& color);
		static void Clear();
		static void SetViewport(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
		static void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray);

	private:
		static RendererAPI* s_RendererAPI;
	};
}