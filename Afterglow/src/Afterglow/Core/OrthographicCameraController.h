#pragma once

#include "Afterglow/Renderer/OrthographicCamera.h"

#include "Afterglow/Core/Timestep.h"
#include "Afterglow/Core/Events/Event.h"
#include "Afterglow/Core/Events/WindowEvents.h"
#include "Afterglow/Core/Events/InputEvents.h"

namespace Afterglow
{
	class OrthographicCameraController
	{
	public:
		OrthographicCameraController(uint16_t width, uint16_t height, bool canRotate = false);
		~OrthographicCameraController() = default;

		void OnUpdate(Timestep ts);
		void OnEvent(Event& event);
		
		void Resize(uint16_t width, uint16_t height);

		OrthographicCamera& GetCamera() { return m_OrthographicCamera; }
		const OrthographicCamera& GetCamera() const { return m_OrthographicCamera; }

	private:
		bool OnWindowResize(WindowResizeEvent& e);
		bool OnMouseScroll(MouseScrolledEvent& e);

	private:
		float m_AspectRatio;
		float m_ZoomLevel = 1.0f;
		OrthographicCamera m_OrthographicCamera;
		
		bool b_CanRotate;
		
		glm::vec3 m_CameraPosition = { 0.0f, 0.0f, 0.0f };
		float m_TranslationSpeed = m_ZoomLevel;

		float m_CameraRotation = 0.0f;
		float m_RotationSpeed = 30.0f;
	};
}