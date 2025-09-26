#pragma once

#include "Afterglow/Renderer/RendererAPI.h"

namespace Afterglow
{
	class OpenGLRendererAPI : public RendererAPI
	{
	public:
		void Init() override;
		void SetClearColor(const glm::vec4& color) override;
		void Clear() override;
		void SetViewport(uint16_t x, uint16_t y, uint16_t width, uint16_t height) override;
		void BindTexture(uint32_t rendererID) override;
		void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray) override;
	};
}