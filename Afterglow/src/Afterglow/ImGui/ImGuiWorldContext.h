#pragma once

#include "Afterglow/Renderer/Renderer2D.h"
#include "Afterglow/Renderer/Framebuffer.h"
#include "Afterglow/Core/OrthographicCameraController.h"

#include <imgui.h>
#include <glm/glm.hpp>

namespace Afterglow
{
	struct WorldPanel
	{
		std::string Name;
		glm::vec2 WorldPosition = { 0.0f, 0.0f };
		glm::vec2 WorldSize = { 1.0f, 1.0f };
		bool Visible = true;
		bool Draggable = true;

		ImGuiWindowFlags WindowFlags = ImGuiWindowFlags_None;

		ImGuiContext* Context = nullptr;
		std::shared_ptr<Framebuffer> Framebuffer = nullptr;

		bool IsDragging = false;
		glm::vec2 DragStartWorldPos = { 0.0f, 0.0f };
		glm::vec2 DragStartPanelPos = { 0.0f, 0.0f };
	};

	class ImGuiWorldContext
	{
	public:
		ImGuiWorldContext() = default;
		~ImGuiWorldContext();

		void Initialize();
		void Shutdown();

		void AddPanel(const std::string& panelName, const glm::vec2& worldPos, const glm::vec2& worldSize, uint16_t framebufferWidth = 512, uint16_t framebufferHeight = 512, ImGuiWindowFlags flags = ImGuiWindowFlags_None, bool draggable = true);
		void RemovePanel(const std::string& panelName);

		void SetPanelPosition(const std::string& panelName, const glm::vec2& position);
		void SetPanelSize(const std::string& panelName, const glm::vec2& size);
		void SetPanelVisible(const std::string& panelName, bool visible);
		void SetPanelDraggable(const std::string& panelName, bool draggable);

		WorldPanel* GetPanel(const std::string& panelName);

		void BeginPanel(const std::string& panelName);
		void EndPanel();

		bool HandleInput(const OrthographicCameraController& camera, const glm::vec2& viewportSize, const glm::vec2& mouseScreen, bool mouseDown);
		void RenderInWorld(Renderer2D& renderer);

	private:
		bool IsPointInWorldBounds(const glm::vec2& worldPoint, const glm::vec2& boundsMin, const glm::vec2& boundsMax) const;
		glm::vec2 ScreenToWorld(const glm::vec2& viewportSize, const glm::vec2& screenPos, const OrthographicCameraController& camera) const;

		void HandlePanelDragging(const glm::vec2& mouseWorld, bool mouseDown);
		std::string FindPanelAtPosition(const glm::vec2& worldPos) const;
		bool IsPointInPanelTitleBar(const glm::vec2& worldPos, const WorldPanel& panel) const;

		glm::vec2 WorldToImGui(const glm::vec2& worldPos, const WorldPanel& panel) const;

	private:
		ImGuiContext* m_ScreenContext = nullptr;
		std::unordered_map<std::string, WorldPanel> m_Panels;

		std::string m_CurrentPanel = "";
		std::string m_DraggingPanel = "";
		std::string m_HoveredPanel = "";
		bool b_IsInteracting = false;
		bool b_IsInitialized = false;

		glm::vec2 m_LastMouseWorld = { 0.0f, 0.0f };
	};
}