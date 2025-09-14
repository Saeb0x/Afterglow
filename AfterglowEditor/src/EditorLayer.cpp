#include "EditorLayer.h"

#include <imgui.h>

EditorLayer::EditorLayer()
	: Afterglow::Layer("EditorLayer"), 
	m_OrthoCameraController(Afterglow::Application::Get().GetWindow().GetWidth(), Afterglow::Application::Get().GetWindow().GetHeight())
{
	m_Pic = Afterglow::Texture2D::Create("assets/textures/pic.jpeg");

	Afterglow::FramebufferSpecification fbSpec;
	fbSpec.Width = 1280;
	fbSpec.Height = 720;

	m_Framebuffer = Afterglow::Framebuffer::Create(fbSpec);
}

void EditorLayer::OnAttach()
{
	AG_APP_TRACE("Layer \"{0}\" is attached", m_DebugName);
}

void EditorLayer::OnDetach()
{
	AG_APP_TRACE("Layer \"{0}\" is detached", m_DebugName);
}

void EditorLayer::OnUpdate(Afterglow::Timestep ts)
{
    if(b_ViewportFocused)
	    m_OrthoCameraController.OnUpdate(ts);

    m_PicRotation += 30.0f * ts;
	m_Framebuffer->Bind();
    m_Renderer2D.ResetStats();
	Afterglow::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
	Afterglow::RenderCommand::Clear();
	m_Renderer2D.BeginScene(m_OrthoCameraController.GetCamera());
	m_Renderer2D.DrawQuad({ 0.0f, -0.5f }, { 0.5f, 0.2f }, { 0.2f, 0.3f, 0.8f, 1.0f });
	m_Renderer2D.DrawQuad({ -0.9f, 0.0f }, { 0.5f, 0.5f }, { 0.8f, 0.2f, 0.3f, 1.0f });
	m_Renderer2D.DrawRotatedQuad({ 0.0f, 0.3f }, { 0.8f, 0.8f }, m_PicRotation, m_Pic);
	m_Renderer2D.EndScene();
	m_Framebuffer->Unbind();
}

void EditorLayer::OnEvent(Afterglow::Event& event)
{
	m_OrthoCameraController.OnEvent(event);
}

void EditorLayer::OnImGuiRender()
{
    static bool opt_fullscreen = true;
    static bool opt_padding = false;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
    // because it would be confusing to have two docking targets within each others.
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    if (opt_fullscreen)
    {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    }
    else
    {
        dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
    }

    // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
    // and handle the pass-thru hole, so we ask Begin() to not render a background.
    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;

    // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
    // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
    // all active windows docked into it will lose their parent and become undocked.
    // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
    // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
    if (!opt_padding)
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace Demo", (bool*)true, window_flags);
    if (!opt_padding)
        ImGui::PopStyleVar();

    if (opt_fullscreen)
        ImGui::PopStyleVar(2);

    // Submit the DockSpace
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    }

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Exit"))
                Afterglow::Application::Get().Close();

            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    ImGui::Begin("Renderer2D Stats");
    ImGui::Text("Draw Calls: %llu", m_Renderer2D.GetStats().DrawCalls);
    ImGui::End();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0,0 });
    ImGui::Begin("Viewport");

    b_ViewportFocused = ImGui::IsWindowFocused();
    b_ViewportHovered = ImGui::IsWindowHovered();
    Afterglow::Application::Get().GetImGuiLayer()->SetConsumeEvents(!b_ViewportFocused || !b_ViewportHovered);

    ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
    if (m_ViewportSize != glm::vec2{ viewportPanelSize.x, viewportPanelSize.y })
    {
        m_Framebuffer->Resize((uint16_t)viewportPanelSize.x, (uint16_t)viewportPanelSize.y);
        m_OrthoCameraController.Resize((uint16_t)viewportPanelSize.x, (uint16_t)viewportPanelSize.y);
        m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };
    }

    uint32_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
    ImGui::Image((unsigned long long)textureID, ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

    ImGui::End();
    ImGui::PopStyleVar();

    ImGui::End();
}