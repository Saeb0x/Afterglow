#pragma once

#include <Afterglow.h>

class EditorLayer : public Afterglow::Layer
{ 
public:
	EditorLayer();
	~EditorLayer() override = default;

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(Afterglow::Timestep ts) override;
	void OnImGuiRender() override;
	void OnEvent(Afterglow::Event& event) override;

private:
	Afterglow::OrthographicCameraController m_OrthoCameraController;

	Afterglow::Renderer2D& m_Renderer2D = Afterglow::Renderer2D::GetInstance();
	std::shared_ptr<Afterglow::Framebuffer> m_Framebuffer;
	glm::vec2 m_ViewportSize = {0.0f, 0.0f};
	bool b_ViewportFocused = false;
	bool b_ViewportHovered = false;

	std::shared_ptr<Afterglow::Texture2D> m_Pic;
	float m_PicRotation = 0.0f;
};