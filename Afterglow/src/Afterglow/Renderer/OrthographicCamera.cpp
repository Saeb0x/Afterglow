#include "agpch.h"
#include "OrthographicCamera.h"

namespace Afterglow
{
	OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top, float nearClip, float farClip)
		: m_Left(left), m_Right(right), m_Bottom(bottom), m_Top(top), m_Near(nearClip), m_Far(farClip)
	{
		RecalculateProjection();
		RecalculateViewMatrix();
	}

	void OrthographicCamera::RecalculateProjection()
	{
		SetProjectionMatrix(glm::ortho(m_Left, m_Right, m_Bottom, m_Top, m_Near, m_Far));
	}

	void OrthographicCamera::SetBounds(float left, float right, float bottom, float top)
	{
		m_Left = left; m_Right = right; m_Bottom = bottom; m_Top = top;
		RecalculateProjection();
	}
}