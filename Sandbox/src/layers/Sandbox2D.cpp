#include "Sandbox2D.h"

#include <imgui.h>

Sandbox2D::Sandbox2D()
	: Afterglow::Layer("Sandbox2D"), 
	m_OrthoCameraController((uint16_t)m_Renderer2D.GetViewport().x, (uint16_t)m_Renderer2D.GetViewport().y, true)
{
	m_Pic = Afterglow::Texture2D::Create("assets/textures/pic.jpeg");
	m_ImGuiWorldContext.Initialize({ 0.0f, 0.0f }, { 2.0f, 2.0f });
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
	// True consumed means that the input is being handled by ImGui world-space context.
	bool consumed = m_ImGuiWorldContext.HandleInput(
		m_OrthoCameraController,
		m_Renderer2D.GetViewport(),
		{ Afterglow::Input::GetMouseX(), Afterglow::Input::GetMouseY() },
		Afterglow::Input::IsMouseButtonPressed(AG_MOUSE_BUTTON_LEFT)
	);

	m_OrthoCameraController.OnUpdate(ts);
	m_PicRotation += 30.0f * ts;

	m_ImGuiWorldContext.Begin();
	{
		ImGui::Begin("WorldSpaceUI", nullptr, ImGuiWindowFlags_None);
		{
			ImGui::Text("World Space UI!");
			ImGui::Text("This UI exists in world coordinates");
			ImGui::Button("World Button");
			if (ImGui::CollapsingHeader("Settings"))
			{
				static float value = 0.5f;
				ImGui::SliderFloat("Some Value", &value, 0.0f, 1.0f);
			}
		}
		ImGui::End();
	}
	m_ImGuiWorldContext.End();

	m_Renderer2D.ResetStats();
	Afterglow::RenderCommand::SetClearColor({ 0.0f, 0.0f, 0.0f, 0.0f });
	Afterglow::RenderCommand::Clear();
	m_Renderer2D.BeginScene(m_OrthoCameraController.GetCamera());

	m_Renderer2D.DrawGrid(1.0f, 0.02f, { 0.6f, 0.6f, 0.6f }, {0.1f, 0.1f, 0.1f});

	m_Renderer2D.DrawQuad({ 0.0f, -0.5f, 1.0f }, { 0.3f, 0.3f }, { 0.2f, 0.3f, 0.8f, 1.0f });
	m_Renderer2D.DrawQuad({ -0.9f, 0.0f, 1.0f }, { 0.3f, 0.3f }, { 0.8f, 0.2f, 0.3f, 1.0f });
	m_Renderer2D.DrawRotatedQuad({ -1.0f, 1.0f, 1.0f }, { 0.5f, 0.5f }, m_PicRotation, m_Pic);
	m_ImGuiWorldContext.RenderInWorld(m_Renderer2D);

	m_Renderer2D.EndScene();
}

void Sandbox2D::OnEvent(Afterglow::Event& event)
{
	m_OrthoCameraController.OnEvent(event);
}

void Sandbox2D::OnImGuiRender()
{
	ImGui::Begin("Renderer2D Stats");
	ImGui::Text("Draw Calls: %llu", m_Renderer2D.GetStats().DrawCalls);
	ImGui::End();
}