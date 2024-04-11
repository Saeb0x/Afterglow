#pragma once

#include <glm/glm.hpp>
#include <memory>

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

				std::shared_ptr<Transform> Copy() const;
				void Copy(const std::shared_ptr<Transform>& transform);
				
				inline void SetPosition(const glm::vec2& position) { m_Position = position; }
				inline glm::vec2& GetPosition() { return m_Position; }
				
				void SetScale(const glm::vec2& scale) { m_Scale = scale; }
				inline glm::vec2& GetScale() { return m_Scale; }

				bool operator==(const Transform& other) const;

			private:
				glm::vec2 m_Position;
				glm::vec2 m_Scale;
			};
		}
	}
}
