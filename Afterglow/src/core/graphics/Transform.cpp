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
		}
	}
}