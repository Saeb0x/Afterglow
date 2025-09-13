#pragma once

#include "OrthographicCamera.h"
#include "Texture.h"
#include "Shader.h"

namespace Afterglow
{
	class Renderer2D
	{
	public:
		static Renderer2D& GetInstance();

		void Init();
		void Shutdown();

		ShaderLibrary& GetShaderLibrary() { return m_ShaderLibrary; }

		void BeginScene(const OrthographicCamera& camera);
		void EndScene();

		void DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
		void DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color);
		void DrawQuad(const glm::vec2& position, const glm::vec2& size, const std::shared_ptr<Texture2D>& texture);
		void DrawQuad(const glm::vec3& position, const glm::vec2& size, const std::shared_ptr<Texture2D>& texture);

		void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec4& color);
		void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const glm::vec4& color);
		void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const std::shared_ptr<Texture2D>& texture);
		void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const std::shared_ptr<Texture2D>& texture);

	private:
		Renderer2D() = default;
		~Renderer2D();
		Renderer2D(const Renderer2D&);
		const Renderer2D& operator=(const Renderer2D&);

	private:
		ShaderLibrary& m_ShaderLibrary = ShaderLibrary::GetInstance();
	};
}