#include "agpch.h"
#include "OrthographicCameraController.h"

#include "Afterglow/Core/Input.h"

namespace Afterglow
{
	OrthographicCameraController::OrthographicCameraController(uint16_t width, uint16_t height, bool canRotate)
		: m_AspectRatio((float)width / (float)height), m_OrthographicCamera(-m_AspectRatio * m_ZoomLevel, m_AspectRatio* m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel, -1.0f, 1.0f), b_CanRotate(canRotate)
	{
	}

	void OrthographicCameraController::OnUpdate(Timestep ts)
	{
		if (Input::IsKeyPressed(AG_KEY_A))
			m_CameraPosition.x -= m_TranslationSpeed * ts;
		else if (Input::IsKeyPressed(AG_KEY_D))
			m_CameraPosition.x += m_TranslationSpeed * ts;

		if (Input::IsKeyPressed(AG_KEY_W))
			m_CameraPosition.y += m_TranslationSpeed * ts;
		else if (Input::IsKeyPressed(AG_KEY_S))
			m_CameraPosition.y -= m_TranslationSpeed * ts;

		m_OrthographicCamera.SetPosition(m_CameraPosition);
		m_TranslationSpeed = m_ZoomLevel;

		if (b_CanRotate)
		{
			if (Input::IsKeyPressed(AG_KEY_Q))
				m_CameraRotation += m_RotationSpeed * ts;
			if (Input::IsKeyPressed(AG_KEY_E))
				m_CameraRotation -= m_RotationSpeed * ts;

			m_OrthographicCamera.SetRotation(glm::vec3(0.0f, 0.0f, m_CameraRotation));
		}
	}

	void OrthographicCameraController::OnEvent(Event& event)
	{
		EventDispatcher disp(event);
		disp.Dispatch<WindowResizeEvent>(AG_BIND_FUNC(OrthographicCameraController::OnWindowResize));
		disp.Dispatch<MouseScrolledEvent>(AG_BIND_FUNC(OrthographicCameraController::OnMouseScroll));
	}

	void OrthographicCameraController::Resize(uint16_t width, uint16_t height)
	{
		m_AspectRatio = (float)width / (float)height;
		m_OrthographicCamera.SetBounds(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
	}

	bool OrthographicCameraController::OnWindowResize(WindowResizeEvent& e)
	{
		Resize(e.GetWidth(), e.GetHeight());
		return false;
	}

	bool OrthographicCameraController::OnMouseScroll(MouseScrolledEvent& e)
	{
		m_ZoomLevel -= e.GetYOffset() * 0.25f;
		m_ZoomLevel = std::max(m_ZoomLevel, 0.25f);

		m_OrthographicCamera.SetBounds(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);

		return false;
	}
}