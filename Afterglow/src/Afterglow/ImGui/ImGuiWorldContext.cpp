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

	void ImGuiWorldContext::Initialize(const glm::vec2& worldPos, const glm::vec2& worldSize, uint16_t framebufferWidth, uint16_t framebufferHeight)
	{
		if (b_IsInitialized)
		{
			AG_ASSERT(false, "WorldSpaceImGuiContext already initialized!");
			return;
		}

		m_WorldPosition = worldPos;
		m_WorldSize = worldSize;

		// Main screen-space context.
		m_ScreenContext = ImGui::GetCurrentContext();

		// New ImGui context for world-space UI.
		m_WorldContext = ImGui::CreateContext(ImGui::GetIO().Fonts);

		FramebufferSpecification fbSpec;
		fbSpec.Width = framebufferWidth;
		fbSpec.Height = framebufferHeight;
		m_Framebuffer = Framebuffer::Create(fbSpec);

		// ImGui initialization for the world-space context.
		ImGui::SetCurrentContext(m_WorldContext);
		ImGuiIO& io = ImGui::GetIO();
		
		io.FontDefault = io.Fonts->AddFontFromFileTTF("assets/fonts/opensans/OpenSans-Bold.ttf", 20.0f);
		Application::GetInstance().GetImGuiLayer()->SetupImGuiStyle();
		
		io.DisplaySize = ImVec2(m_Framebuffer->GetSpecification().Width, m_Framebuffer->GetSpecification().Height);
		io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

		ImGui_ImplOpenGL3_Init("#version 330");

		ImGui::SetCurrentContext(m_ScreenContext);

		b_IsInitialized = true;
		AG_INFO("WorldSpaceImGuiContext initialized at world position ({}, {}) with size ({}, {})", worldPos.x, worldPos.y, worldSize.x, worldSize.y);
	}

	void ImGuiWorldContext::Shutdown()
	{
		if (!b_IsInitialized)
			return;

		if (m_WorldContext)
		{
			ImGui::SetCurrentContext(m_WorldContext);
			ImGui_ImplOpenGL3_Shutdown();
			ImGui::DestroyContext(m_WorldContext);
			m_WorldContext = nullptr;
		}

		if (m_ScreenContext)
		{
			ImGui::SetCurrentContext(m_ScreenContext);
		}

		m_Framebuffer = nullptr;
		b_IsInitialized = false;
	}

	void ImGuiWorldContext::Begin()
	{
		if (!b_IsInitialized || !m_Framebuffer)
			return;

		ImGui::SetCurrentContext(m_WorldContext);
		m_Framebuffer->Bind();

		// Clear with a transparent background.
		RenderCommand::SetClearColor({ 0.0f, 0.0f, 0.0f, 0.0f });
		RenderCommand::Clear();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui::NewFrame();
	}

	void ImGuiWorldContext::End()
	{
		if (!b_IsInitialized || !m_Framebuffer)
			return;

		// Render ImGui draw data to a framebuffer.
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		m_Framebuffer->Unbind();

		ImGui::SetCurrentContext(m_ScreenContext);
	}

	bool ImGuiWorldContext::HandleInput(const OrthographicCameraController& camera, const glm::vec2& ViewportSize, const glm::vec2& mouseScreen, bool mouseDown)
	{
		if (!b_IsInitialized)
			return false;

		glm::vec2 mouseWorld = ScreenToWorld(ViewportSize, mouseScreen, camera);
		bool inBounds = IsPointInWorldBounds(mouseWorld);

		if (mouseDown)
		{
			AG_TRACE("=== DEBUG ===");
			AG_TRACE("Mouse Screen: ({:.2f}, {:.2f})", mouseScreen.x, mouseScreen.y);
			AG_TRACE("Mouse World: ({:.2f}, {:.2f})", mouseWorld.x, mouseWorld.y);
			AG_TRACE("World Position: ({:.2f}, {:.2f})", m_WorldPosition.x, m_WorldPosition.y);
			AG_TRACE("World Size: ({:.2f}, {:.2f})", m_WorldSize.x, m_WorldSize.y);
			AG_TRACE("Bounds: X[{:.2f} to {:.2f}], Y[{:.2f} to {:.2f}]", m_WorldPosition.x, m_WorldPosition.x + m_WorldSize.x, m_WorldPosition.y, m_WorldPosition.y + m_WorldSize.y);
			AG_TRACE("In Bounds: {}", inBounds ? "YES" : "NO");
			AG_TRACE("=============");
		}

		// Check if mouse is within our world-space UI bounds.
		if (inBounds)
		{
			ImGui::SetCurrentContext(m_WorldContext);
			ImGuiIO& io = ImGui::GetIO();

			// Convert world coordinates to framebuffer local coordinates.
			glm::vec2 framebufferPos = WorldToFramebuffer(mouseWorld);

			io.MousePos = ImVec2(framebufferPos.x, framebufferPos.y);
			io.MouseDown[0] = mouseDown;

			ImGui::SetCurrentContext(m_ScreenContext);
			b_IsInteracting = true;

			return true; // Input consumed by world-space UI.
		}

		// Reset interaction state when mouse is released outside bounds.
		if (b_IsInteracting && !mouseDown)
		{
			b_IsInteracting = false;

			// Clear mouse state in world-space context.
			ImGui::SetCurrentContext(m_WorldContext);
			ImGuiIO& io = ImGui::GetIO();
			io.MouseDown[0] = false;
			io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
			ImGui::SetCurrentContext(m_ScreenContext);
		}

		return b_IsInteracting; // True if we're still dragging something.
	}

	void ImGuiWorldContext::RenderInWorld(Renderer2D& renderer)
	{
		if (!b_IsInitialized || !m_Framebuffer)
			return;

		// Calculate the center position of the world-space UI quad.
		glm::vec3 centerPosition = glm::vec3(
			m_WorldPosition.x + m_WorldSize.x * 0.5f,
			m_WorldPosition.y + m_WorldSize.y * 0.5f,
			1.0f  // Z-depth for layering.
		);

		// Render the framebuffer as a textured quad in world space. Base quad is 2 units (-1 to 1), so divide by 2 to get actual world size.
		uint32_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
		renderer.DrawQuad(centerPosition, m_WorldSize * 0.5f, textureID);
	}

	void ImGuiWorldContext::ResizeFramebuffer(uint16_t width, uint16_t height)
	{
		if (!b_IsInitialized || !m_Framebuffer)
			return;

		m_Framebuffer->Resize(width, height);

		ImGui::SetCurrentContext(m_WorldContext);
		ImGui::GetIO().DisplaySize = ImVec2((float)width, (float)height);
		ImGui::SetCurrentContext(m_ScreenContext);
	}

	bool ImGuiWorldContext::IsPointInWorldBounds(const glm::vec2& worldPoint) const
	{
		return (worldPoint.x >= m_WorldPosition.x &&
			worldPoint.x <= m_WorldPosition.x + m_WorldSize.x &&
			worldPoint.y >= m_WorldPosition.y &&
			worldPoint.y <= m_WorldPosition.y + m_WorldSize.y);
	}

	glm::vec2 ImGuiWorldContext::ScreenToWorld(const glm::vec2& canvasSize, const glm::vec2& screenPos, const OrthographicCameraController& camera) const
	{
		// Convert screen coordinates to normalized device coordinates(-1 to 1).
		glm::vec2 NDC;
		NDC.x = (2.0f * screenPos.x) / canvasSize.x - 1.0f;
		NDC.y = 1.0f - (2.0f * screenPos.y) / canvasSize.y; // Flip Y axis.

		// Convert NDC to world coordinates using inverse view-projection matrix.
		glm::mat4 invViewProj = glm::inverse(camera.GetCamera().GetProjectionViewMatrix());
		glm::vec4 worldPos4 = invViewProj * glm::vec4(NDC.x, NDC.y, 0.0f, 1.0f);

		return glm::vec2(worldPos4.x, worldPos4.y);
	}

	glm::vec2 ImGuiWorldContext::WorldToFramebuffer(const glm::vec2& worldPos) const
	{
		// Convert world position to normalized coordinates within our UI bounds (0-1).
		glm::vec2 normalized = (worldPos - m_WorldPosition) / m_WorldSize;

		// To framebuffer pixel coordinates.
		glm::vec2 framebufferPos;
		framebufferPos.x = normalized.x * m_Framebuffer->GetSpecification().Width;
		framebufferPos.y = (1.0f - normalized.y) * m_Framebuffer->GetSpecification().Height;

		// Clamp to framebuffer bounds.
		framebufferPos.x = glm::clamp(framebufferPos.x, 0.0f, (float)m_Framebuffer->GetSpecification().Width);
		framebufferPos.y = glm::clamp(framebufferPos.y, 0.0f, (float)m_Framebuffer->GetSpecification().Height);

		return framebufferPos;
	}
}