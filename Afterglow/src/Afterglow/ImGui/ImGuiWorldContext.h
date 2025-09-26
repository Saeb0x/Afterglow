#pragma once

#include "Afterglow/Renderer/Renderer2D.h"
#include "Afterglow/Renderer/Framebuffer.h"
#include "Afterglow/Renderer/OrthographicCamera.h"

#include <imgui.h>
#include <glm/glm.hpp>

namespace Afterglow
{
	class ImGuiWorldContext
	{
	public:
		ImGuiWorldContext() = default;
		~ImGuiWorldContext();

		void Initialize(const glm::vec2& worldPos, const glm::vec2& worldSize, uint16_t framebufferWidth = 512, uint16_t framebufferHeight = 512);
		void Shutdown();

		void Begin();
		void End();

		bool HandleInput(const OrthographicCamera& camera, const glm::vec2& viewportSize, const glm::vec2& mouseScreen, bool mouseDown);
		void RenderInWorld(Renderer2D& renderer);

		void SetWorldPosition(const glm::vec2& position) { m_WorldPosition = position; }
		const glm::vec2& GetWorldPosition() const { return m_WorldPosition; }

		void SetWorldSize(const glm::vec2& size) { m_WorldSize = size; ResizeFramebuffer((uint16_t)size.x, (uint16_t)size.y); }
		const glm::vec2& GetWorldSize() const { return m_WorldSize; }

		void ResizeFramebuffer(uint16_t width, uint16_t height);

	private:
		bool IsPointInWorldBounds(const glm::vec2& worldPoint) const;
		glm::vec2 ScreenToWorld(const glm::vec2& viewportSize, const glm::vec2& screenPos, const OrthographicCamera& camera) const;
		glm::vec2 WorldToFramebuffer(const glm::vec2& worldPos) const;

	private:
		ImGuiContext* m_WorldContext = nullptr;
		ImGuiContext* m_ScreenContext = nullptr;

		std::shared_ptr<Framebuffer> m_Framebuffer = nullptr;

		glm::vec2 m_WorldPosition = { 0.0f, 0.0f };
		glm::vec2 m_WorldSize = { 1.0f, 1.0f };

		bool b_IsInteracting = false;
		bool b_IsInitialized = false;
	};
}