#pragma once

#include "Camera.h"

namespace Afterglow
{
	class OrthographicCamera : public Camera
	{
	public:
		OrthographicCamera(float left, float right, float bottom, float top, float nearClip, float farClip);
		~OrthographicCamera() override = default;

		void RecalculateProjection() override;

		void SetBounds(float left, float right, float bottom, float top);

		float GetLeft() const { return m_Left; }
		float GetRight() const { return m_Right; }
		float GetBottom() const { return m_Bottom; }
		float GetTop() const { return m_Top; }

	private:
		float m_Left, m_Right, m_Bottom, m_Top;
		float m_Near, m_Far;
	};
}