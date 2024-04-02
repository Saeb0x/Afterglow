#include "Transform.h"

namespace Afterglow
{
	namespace Core
	{
		namespace Graphics
		{
			Transform::Transform()
			{
				Init(glm::vec2({ 0.0f, 0.0f }), glm::vec2({ 0.0f, 0.0f }));
			}

			Transform::Transform(const glm::vec2& position)
			{
				Init(position, glm::vec2({ 0.0f, 0.0f }));
			}

			Transform::Transform(const glm::vec2& position, const glm::vec2& scale)
			{
				Init(position, scale);
			}

			Transform::~Transform() {}

			void Transform::Init(const glm::vec2& position, const glm::vec2& scale)
			{
				m_Position = position;
				m_Scale = scale;
			}

			std::shared_ptr<Transform> Transform::Copy() const
			{
				return std::make_shared<Transform>(*this);
			}

			void Transform::Copy(const std::shared_ptr<Transform>& transform)
			{
				*transform = *this;
			}
			bool Transform::operator==(const Transform& other) const
			{
				return m_Position == other.m_Position && m_Scale == other.m_Scale;
			}
		}
	}
}