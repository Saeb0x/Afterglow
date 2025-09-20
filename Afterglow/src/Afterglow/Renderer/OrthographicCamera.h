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

	private:
		float m_Left, m_Right, m_Bottom, m_Top;
		float m_Near, m_Far;
	};
}