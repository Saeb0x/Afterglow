#include "Sandbox2D.h"

#include <imgui.h>

Sandbox2D::Sandbox2D()
	: Afterglow::Layer("Sandbox2D"), 
	m_OrthoCameraController(Afterglow::Application::Get().GetWindow().GetWidth(), Afterglow::Application::Get().GetWindow().GetHeight())
{
	m_Pic = Afterglow::Texture2D::Create("assets/textures/pic.jpeg");

	Afterglow::FramebufferSpecification fbSpec;
	fbSpec.Width = 1280;
	fbSpec.Height = 720;

	m_Framebuffer = Afterglow::Framebuffer::Create(fbSpec);
}

void Sandbox2D::OnAttach()
{
	AG_APP_TRACE("Layer \"{0}\" is attached", m_DebugName);
}

void Sandbox2D::OnDetach()
{
	AG_APP_TRACE("Layer \"{0}\" is detached", m_DebugName);
}

void Sandbox2D::OnUpdate(Afterglow::Timestep ts)
{
	m_OrthoCameraController.OnUpdate(ts);
	m_PicRotation += 30.0f * ts;

	m_Framebuffer->Bind();
	Afterglow::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
	Afterglow::RenderCommand::Clear();
	m_Renderer2D.BeginScene(m_OrthoCameraController.GetCamera());
	m_Renderer2D.DrawQuad({ 0.0f, -0.5f }, { 0.5f, 0.2f }, { 0.2f, 0.3f, 0.8f, 1.0f });
	m_Renderer2D.DrawQuad({ -0.9f, 0.0f }, { 0.5f, 0.5f }, { 0.8f, 0.2f, 0.3f, 1.0f });
	m_Renderer2D.DrawRotatedQuad({ 0.0f, 0.3f }, { 0.8f, 0.8f }, m_PicRotation, m_Pic);
	m_Renderer2D.EndScene();
	m_Framebuffer->Unbind();
}

void Sandbox2D::OnEvent(Afterglow::Event& event)
{
	m_OrthoCameraController.OnEvent(event);
}

void Sandbox2D::OnImGuiRender()
{
	uint32_t textureID = m_Framebuffer->GetColorAttachmentRendererID();

	static bool ViewportOpen = true;
	ImGui::Begin("Viewport", &ViewportOpen);
	ImGui::Image((ImTextureID)textureID, 
		ImVec2{1280, 720},
		ImVec2{0, 1}, // UV0: top-left in OpenGL space.
		ImVec2{1, 0}  // UV1: bottom-right in OpenGL space.
	);
	ImGui::End();
}