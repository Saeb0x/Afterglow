#pragma once

#include <glm/glm.hpp>

namespace Afterglow
{
	namespace Core
	{
		namespace Graphics
		{
			class Transform
			{
			public:
				Transform();
				Transform(const glm::vec2& position);
				Transform(const glm::vec2& position, const glm::vec2& scale);
				~Transform();

				void Init(const glm::vec2& position, const glm::vec2& scale);

				inline const glm::vec2& GetPosition() const { return m_Position; }
				inline const glm::vec2& GetScale() const { return m_Scale; }
			private:
				glm::vec2 m_Position;
				glm::vec2 m_Scale;
			};
		}
	}
}
