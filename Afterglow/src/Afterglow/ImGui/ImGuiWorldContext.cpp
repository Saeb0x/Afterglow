#include "agpch.h"
#include "ImGuiWorldContext.h"

#include "Afterglow/Core/Application.h"

#include <backends/imgui_impl_opengl3.h>

namespace Afterglow
{
	ImGuiWorldContext::~ImGuiWorldContext()
	{
		Shutdown();
	}

	void ImGuiWorldContext::Initialize()
	{
		if (b_IsInitialized)
		{
			AG_ASSERT(false, "WorldSpaceImGuiContext already initialized!");
			return;
		}

		m_ScreenContext = ImGui::GetCurrentContext();

		b_IsInitialized = true;
		AG_INFO("WorldSpaceImGuiContext initialized");
	}

	void ImGuiWorldContext::Shutdown()
	{
		if (!b_IsInitialized)
			return;

		for (auto& [name, panel] : m_Panels)
		{
			if (panel.Context)
			{
				ImGui::SetCurrentContext(panel.Context);
				ImGui_ImplOpenGL3_Shutdown();
				ImGui::DestroyContext(panel.Context);
			}
		}

		if (m_ScreenContext)
		{
			ImGui::SetCurrentContext(m_ScreenContext);
		}

		m_Panels.clear();
		b_IsInitialized = false;
	}

	void ImGuiWorldContext::AddPanel(const std::string& panelName, const glm::vec2& worldPos, const glm::vec2& worldSize, uint16_t framebufferWidth, uint16_t framebufferHeight, ImGuiWindowFlags flags, bool draggable)
	{
		WorldPanel panel;
		panel.Name = panelName;
		panel.WorldPosition = worldPos;
		panel.WorldSize = worldSize;
		panel.WindowFlags = flags;
		panel.Visible = true;
		panel.Draggable = draggable;
		panel.IsDragging = false;

		FramebufferSpecification fbSpec;
		fbSpec.Width = framebufferWidth;
		fbSpec.Height = framebufferHeight;
		panel.Framebuffer = Framebuffer::Create(fbSpec);

		// Unique ImGui context for this panel.
		panel.Context = ImGui::CreateContext(ImGui::GetIO().Fonts);

		// ImGui initialization for the panel's context.
		ImGui::SetCurrentContext(panel.Context);
		ImGuiIO& io = ImGui::GetIO();

		io.FontDefault = io.Fonts->AddFontFromFileTTF("assets/fonts/opensans/OpenSans-Bold.ttf", 20.0f);
		Application::GetInstance().GetImGuiLayer()->SetupImGuiStyle();

		io.DisplaySize = ImVec2((float)framebufferWidth, (float)framebufferHeight);
		io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
		io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

		ImGui_ImplOpenGL3_Init("#version 330");

		ImGui::SetCurrentContext(m_ScreenContext);

		m_Panels[panelName] = panel;
		AG_INFO("Added world panel '{}' at ({}, {}) with size ({}, {}) and resolution {}x{}", panelName, worldPos.x, worldPos.y, worldSize.x, worldSize.y, framebufferWidth, framebufferHeight);
	}

	void ImGuiWorldContext::RemovePanel(const std::string& panelName)
	{
		auto it = m_Panels.find(panelName);
		if (it != m_Panels.end())
		{
			if (it->second.Context)
			{
				ImGui::SetCurrentContext(it->second.Context);
				ImGui_ImplOpenGL3_Shutdown();
				ImGui::DestroyContext(it->second.Context);
			}
			m_Panels.erase(it);
		}
	}

	void ImGuiWorldContext::SetPanelPosition(const std::string& panelName, const glm::vec2& position)
	{
		if (auto it = m_Panels.find(panelName); it != m_Panels.end())
			it->second.WorldPosition = position;
	}

	void ImGuiWorldContext::SetPanelSize(const std::string& panelName, const glm::vec2& size)
	{
		if (auto it = m_Panels.find(panelName); it != m_Panels.end())
			it->second.WorldSize = size;
	}

	void ImGuiWorldContext::SetPanelVisible(const std::string& panelName, bool visible)
	{
		if (auto it = m_Panels.find(panelName); it != m_Panels.end())
			it->second.Visible = visible;
	}

	void ImGuiWorldContext::SetPanelDraggable(const std::string& panelName, bool draggable)
	{
		if (auto it = m_Panels.find(panelName); it != m_Panels.end())
			it->second.Draggable = draggable;
	}

	WorldPanel* ImGuiWorldContext::GetPanel(const std::string& panelName)
	{
		auto it = m_Panels.find(panelName);
		return (it != m_Panels.end()) ? &it->second : nullptr;
	}

	void ImGuiWorldContext::BeginPanel(const std::string& panelName)
	{
		if (!b_IsInitialized)
			return;

		auto it = m_Panels.find(panelName);
		if (it == m_Panels.end() || !it->second.Visible || !it->second.Framebuffer || !it->second.Context)
			return;

		m_CurrentPanel = panelName;
		WorldPanel& panel = it->second;

		ImGui::SetCurrentContext(panel.Context);

		panel.Framebuffer->Bind();

		// Clear with a transparent background.
		RenderCommand::SetClearColor({ 0.0f, 0.0f, 0.0f, 0.0f });
		RenderCommand::Clear();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui::NewFrame();

		ImGuiIO& io = ImGui::GetIO();
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(io.DisplaySize);

		ImGuiWindowFlags finalFlags = panel.WindowFlags |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoCollapse;

		ImGui::Begin(panel.Name.c_str(), nullptr, finalFlags);
	}

	void ImGuiWorldContext::EndPanel()
	{
		if (m_CurrentPanel.empty())
			return;

		auto it = m_Panels.find(m_CurrentPanel);
		if (it == m_Panels.end() || !it->second.Framebuffer || !it->second.Context)
			return;

		ImGui::End();

		// Render ImGui draw data to the framebuffer.
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		it->second.Framebuffer->Unbind();

		ImGui::SetCurrentContext(m_ScreenContext);
		m_CurrentPanel.clear();
	}

	bool ImGuiWorldContext::HandleInput(const OrthographicCameraController& camera, const glm::vec2& viewportSize,
		const glm::vec2& mouseScreen, bool mouseDown)
	{
		if (!b_IsInitialized)
			return false;

		glm::vec2 mouseWorld = ScreenToWorld(viewportSize, mouseScreen, camera);
		m_LastMouseWorld = mouseWorld;

		HandlePanelDragging(mouseWorld, mouseDown);

		// If we're dragging a panel, clear input for all panels.
		if (!m_DraggingPanel.empty())
		{
			for (auto& [name, panel] : m_Panels)
			{
				if (!panel.Context)
					continue;

				ImGui::SetCurrentContext(panel.Context);
				ImGuiIO& io = ImGui::GetIO();
				io.MouseDown[0] = false;
				io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
			}
			ImGui::SetCurrentContext(m_ScreenContext);
			return true;
		}

		std::string hoveredPanel = FindPanelAtPosition(mouseWorld);
		m_HoveredPanel = hoveredPanel;

		for (auto& [name, panel] : m_Panels)
		{
			if (!panel.Context || !panel.Visible)
				continue;

			ImGui::SetCurrentContext(panel.Context);
			ImGuiIO& io = ImGui::GetIO();

			if (name == hoveredPanel)
			{
				// Convert world coordinates to framebuffer local coordinates.
				glm::vec2 panelLocal = WorldToImGui(mouseWorld, panel);
				io.MousePos = ImVec2(panelLocal.x, panelLocal.y);
				io.MouseDown[0] = mouseDown;
			}
			else
			{
				io.MouseDown[0] = false;
				io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
			}
		}

		ImGui::SetCurrentContext(m_ScreenContext);

		bool inAnyPanel = !hoveredPanel.empty();

		if (mouseDown && inAnyPanel)
			b_IsInteracting = true;
		else if (!mouseDown)
			b_IsInteracting = false;

		return b_IsInteracting || inAnyPanel;
	}

	void ImGuiWorldContext::HandlePanelDragging(const glm::vec2& mouseWorld, bool mouseDown)
	{
		if (mouseDown && m_DraggingPanel.empty() && !b_IsInteracting)
		{
			for (auto& [name, panel] : m_Panels)
			{
				if (!panel.Visible || !panel.Draggable || !panel.Context)
					continue;

				glm::vec2 panelMin = panel.WorldPosition;
				glm::vec2 panelMax = panel.WorldPosition + panel.WorldSize;

				if (!IsPointInWorldBounds(mouseWorld, panelMin, panelMax))
					continue;

				ImGui::SetCurrentContext(panel.Context);
				ImGuiIO& io = ImGui::GetIO();

				// If ImGui wants the mouse (hovering over an interactive element), don't drag.
				bool wantsMouseCapture = ImGui::IsAnyItemHovered() || ImGui::IsAnyItemActive() || ImGui::IsAnyItemFocused();

				ImGui::SetCurrentContext(m_ScreenContext);

				if (wantsMouseCapture)
				{
					continue;
				}

				m_DraggingPanel = name;
				panel.IsDragging = true;
				panel.DragStartWorldPos = mouseWorld;
				panel.DragStartPanelPos = panel.WorldPosition;
				AG_TRACE("Started dragging panel '{}'", name);
				break;
			}
		}
		else if (mouseDown && !m_DraggingPanel.empty())
		{
			auto it = m_Panels.find(m_DraggingPanel);
			if (it != m_Panels.end())
			{
				WorldPanel& panel = it->second;
				glm::vec2 dragDelta = mouseWorld - panel.DragStartWorldPos;
				panel.WorldPosition = panel.DragStartPanelPos + dragDelta;
			}
		}
		else if (!mouseDown && !m_DraggingPanel.empty())
		{
			auto it = m_Panels.find(m_DraggingPanel);
			if (it != m_Panels.end())
			{
				it->second.IsDragging = false;
				AG_TRACE("Stopped dragging panel '{}'", m_DraggingPanel);
			}
			m_DraggingPanel.clear();
		}
	}

	bool ImGuiWorldContext::IsPointInPanelTitleBar(const glm::vec2& worldPos, const WorldPanel& panel) const
	{
		if (!panel.Framebuffer)
			return false;

		float titleBarWorldHeight = (30.0f / panel.Framebuffer->GetSpecification().Height) * panel.WorldSize.y;

		glm::vec2 titleBarMin = glm::vec2(
			panel.WorldPosition.x,
			panel.WorldPosition.y + panel.WorldSize.y - titleBarWorldHeight
		);
		glm::vec2 titleBarMax = glm::vec2(
			panel.WorldPosition.x + panel.WorldSize.x,
			panel.WorldPosition.y + panel.WorldSize.y
		);

		return IsPointInWorldBounds(worldPos, titleBarMin, titleBarMax);
	}

	std::string ImGuiWorldContext::FindPanelAtPosition(const glm::vec2& worldPos) const
	{
		for (const auto& [name, panel] : m_Panels)
		{
			if (!panel.Visible)
				continue;

			glm::vec2 panelMin = panel.WorldPosition;
			glm::vec2 panelMax = panel.WorldPosition + panel.WorldSize;

			if (IsPointInWorldBounds(worldPos, panelMin, panelMax))
			{
				return name;
			}
		}
		return "";
	}

	void ImGuiWorldContext::RenderInWorld(Renderer2D& renderer)
	{
		if (!b_IsInitialized)
			return;

		for (const auto& [name, panel] : m_Panels)
		{
			if (!panel.Visible || !panel.Framebuffer)
				continue;

			// Calculate the center position of the world-space panel quad.
			glm::vec3 centerPosition = glm::vec3(
				panel.WorldPosition.x + panel.WorldSize.x * 0.5f,
				panel.WorldPosition.y + panel.WorldSize.y * 0.5f,
				1.0f  // Z-depth for layering.
			);

			// Render the framebuffer as a textured quad in world space.
			uint32_t textureID = panel.Framebuffer->GetColorAttachmentRendererID();
			renderer.DrawQuad(centerPosition, panel.WorldSize * 0.5f, textureID);
		}
	}

	bool ImGuiWorldContext::IsPointInWorldBounds(const glm::vec2& worldPoint, const glm::vec2& boundsMin, const glm::vec2& boundsMax) const
	{
		return (worldPoint.x >= boundsMin.x && worldPoint.x <= boundsMax.x &&
			worldPoint.y >= boundsMin.y && worldPoint.y <= boundsMax.y);
	}

	glm::vec2 ImGuiWorldContext::ScreenToWorld(const glm::vec2& canvasSize, const glm::vec2& screenPos,
		const OrthographicCameraController& camera) const
	{
		// Convert screen coordinates to normalized device coordinates (-1 to 1).
		glm::vec2 NDC;
		NDC.x = (2.0f * screenPos.x) / canvasSize.x - 1.0f;
		NDC.y = 1.0f - (2.0f * screenPos.y) / canvasSize.y; // Flip Y axis.

		// Convert NDC to world coordinates using inverse view-projection matrix.
		glm::mat4 invViewProj = glm::inverse(camera.GetCamera().GetProjectionViewMatrix());
		glm::vec4 worldPos4 = invViewProj * glm::vec4(NDC.x, NDC.y, 0.0f, 1.0f);

		return glm::vec2(worldPos4.x, worldPos4.y);
	}

	glm::vec2 ImGuiWorldContext::WorldToImGui(const glm::vec2& worldPos, const WorldPanel& panel) const
	{
		if (!panel.Framebuffer)
			return glm::vec2(0.0f, 0.0f);

		// Convert world position to normalized coordinates within panel bounds (0-1).
		glm::vec2 panelLocal = worldPos - panel.WorldPosition;
		glm::vec2 normalized = panelLocal / panel.WorldSize;

		// To framebuffer pixel coordinates.
		glm::vec2 imguiPos;
		imguiPos.x = normalized.x * panel.Framebuffer->GetSpecification().Width;
		imguiPos.y = (1.0f - normalized.y) * panel.Framebuffer->GetSpecification().Height;

		return imguiPos;
	}
}