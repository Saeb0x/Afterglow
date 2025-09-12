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
	Afterglow::OrthographicCameraController m_OrthoCameraController;

	std::shared_ptr<Afterglow::VertexArray> m_VertexArray;
	std::shared_ptr<Afterglow::VertexBuffer> m_VertexBuffer;
	std::shared_ptr<Afterglow::IndexBuffer> m_IndexBuffer;
	std::shared_ptr<Afterglow::Shader> m_Shader;
	std::shared_ptr<Afterglow::Texture2D> m_Texture;
};