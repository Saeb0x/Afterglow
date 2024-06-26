#include "OrthographicCamera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Afterglow
{
	namespace Core
	{
		OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top) :
			m_ProjectionMatrix(glm::ortho(left, right, bottom, top, -1.0f, 1.0f)), m_ViewMatrix(glm::mat4(1.0f))
		{

			m_ProjectionViewMatrix = m_ProjectionMatrix * m_ViewMatrix;
		}

		OrthographicCamera::~OrthographicCamera() {}

		void OrthographicCamera::RecalculateViewMatrix()
		{
			glm::mat4 transform = glm::rotate(glm::mat4(1.0f), glm::degrees(m_Rotation), glm::vec3(0, 0, 1)) * glm::translate(glm::mat4(1.0f), m_Position);
			m_ViewMatrix = glm::inverse(transform);
			m_ProjectionViewMatrix = m_ProjectionMatrix * m_ViewMatrix;
		}
	}
}