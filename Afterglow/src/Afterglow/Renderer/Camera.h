#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>

namespace Afterglow
{
	class Camera
	{
	public:
		Camera() = default;
		virtual ~Camera() = default;

		virtual void RecalculateProjection() = 0;

		const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
		const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		const glm::mat4& GetProjectionViewMatrix() const { return m_ProjectionViewMatrix; }

		void SetPosition(const glm::vec3& pos) { m_Position = pos; RecalculateViewMatrix(); }
		const glm::vec3& GetPosition() const { return m_Position; }

		void SetRotation(const glm::vec3& rot) { m_Rotation = rot; RecalculateViewMatrix(); }
		const glm::vec3& GetRotation() const { return m_Rotation; }

	protected:
		void RecalculateViewMatrix()
		{
			glm::mat4 transform = glm::yawPitchRoll(glm::radians(m_Rotation.y), glm::radians(m_Rotation.x), glm::radians(m_Rotation.z)) * glm::translate(glm::mat4(1.0), m_Position);

			m_ViewMatrix = glm::inverse(transform);

			m_ProjectionViewMatrix = m_ProjectionMatrix * m_ViewMatrix;
		}

		void SetProjectionMatrix(const glm::mat4& proj)
		{
			m_ProjectionMatrix = proj;
			m_ProjectionViewMatrix = m_ProjectionMatrix * m_ViewMatrix;
		}

	private:
		glm::mat4 m_ProjectionMatrix{1.0f};
		glm::mat4 m_ViewMatrix{1.0f};
		glm::mat4 m_ProjectionViewMatrix{1.0f};

		glm::vec3 m_Position{0.0f};
		glm::vec3 m_Rotation{0.0f}; // Pitch, Yaw, Roll.
	};
}