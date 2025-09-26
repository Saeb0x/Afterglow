#pragma once

#include "RenderCommand.h"
#include "Camera.h"
#include "Texture.h"
#include "Shader.h"

namespace Afterglow
{
	struct Stats
	{
		uint32_t DrawCalls = 0;
	};

	class Renderer2D
	{
	public:
		static Renderer2D& GetInstance();

		static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }

		void Init();
		void Shutdown();

		const Stats& GetStats() const { return m_Stats; }
		void ResetStats();

		void SetViewport(uint16_t x, uint16_t y, uint16_t width, uint16_t height);

		ShaderLibrary& GetShaderLibrary() { return m_ShaderLibrary; }

		void BeginScene(const Camera& camera);
		void EndScene();

		void DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
		void DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color);
		void DrawQuad(const glm::vec2& position, const glm::vec2& size, const std::shared_ptr<Texture2D>& texture);
		void DrawQuad(const glm::vec3& position, const glm::vec2& size, const std::shared_ptr<Texture2D>& texture);
		void DrawQuad(const glm::vec2& position, const glm::vec2& size, uint32_t rendererID);
		void DrawQuad(const glm::vec3& position, const glm::vec2& size, uint32_t rendererID);

		void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec4& color);
		void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const glm::vec4& color);
		void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const std::shared_ptr<Texture2D>& texture);
		void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const std::shared_ptr<Texture2D>& texture);

		void DrawGrid(float spacing, float thickness, const glm::vec3& gridColor, const glm::vec3& backgroundColor);

	private:
		Renderer2D() = default;
		~Renderer2D() = default;

		Renderer2D(const Renderer2D&) = delete;
		Renderer2D& operator=(const Renderer2D&) = delete;
		Renderer2D(Renderer2D&&) = delete;
		Renderer2D& operator=(Renderer2D&&) = delete;

	private:
		ShaderLibrary& m_ShaderLibrary = ShaderLibrary::GetInstance();
		Stats m_Stats;
	};
}