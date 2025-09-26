#pragma once

#include <Afterglow.h>

class Sandbox2D : public Afterglow::Layer
{ 
public:
	Sandbox2D();
	~Sandbox2D() override = default;

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(Afterglow::Timestep ts) override;
	void OnImGuiRender() override;
	void OnEvent(Afterglow::Event& event) override;

private:
	Afterglow::Renderer2D& m_Renderer2D = Afterglow::Renderer2D::GetInstance();
	glm::vec2 m_Viewport;

	Afterglow::OrthographicCameraController m_OrthoCameraController;

	std::shared_ptr<Afterglow::Texture2D> m_Pic;
	float m_PicRotation = 0.0f;

	Afterglow::ImGuiWorldContext m_ImGuiWorldContext;
};