#pragma once

#include <glm/glm.hpp>

namespace Afterglow
{
	namespace Core
	{
		class OrthographicCamera
		{
		public:
			OrthographicCamera(float left, float right, float bottom, float top);
			~OrthographicCamera();

			inline const glm::vec3& GetPosition() const { return m_Position; }
			inline void SetPosition(const glm::vec3& position) { m_Position = position; RecalculateViewMatrix(); }

			inline float GetRotation() const { return m_Rotation; }
			inline void SetRotation(float rotation) { m_Rotation = rotation; RecalculateViewMatrix();}

			inline const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
			inline const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
			inline const glm::mat4& GetProjectionViewMatrix() const { return m_ProjectionViewMatrix; }

			inline glm::mat4 GetInverseProjectionMatrix() const { return glm::inverse(m_ProjectionMatrix); }
			inline glm::mat4 GetInverseViewMatrix() const { return glm::inverse(m_ViewMatrix); }
		private:
			void RecalculateViewMatrix();
		private:
			glm::mat4 m_ProjectionMatrix;
			glm::mat4 m_ViewMatrix;
			glm::mat4 m_ProjectionViewMatrix;

			glm::vec3 m_Position = {0.0f, 0.0f, 0.0f};
			float m_Rotation = 0.0f;
		};
	}
}
