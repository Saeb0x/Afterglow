#include "VertexLayout.h"

namespace Afterglow
{
	namespace Core
	{
		namespace Graphics
		{
			VertexLayout::VertexLayout() : m_Stride(0), m_Elements({}) {}
			VertexLayout::~VertexLayout() {}

			template<typename T>
			void VertexLayout::Push(unsigned int count)
			{
				static_assert(std::is_same_v<T, float> || std::is_same_v<T, unsigned int> || std::is_same_v<T, unsigned char>, "Unsupported data type for buffer layout");
			}

			template<>
			void VertexLayout::Push<float>(unsigned int count)
			{
				m_Elements.push_back({ GL_FLOAT, count, false });
				m_Stride += count * LayoutElement::GetSizeOfType(GL_FLOAT);
			}

			template<>
			void VertexLayout::Push<unsigned int>(unsigned int count)
			{
				m_Elements.push_back({ GL_UNSIGNED_INT, count, false });
				m_Stride += count * LayoutElement::GetSizeOfType(GL_UNSIGNED_INT);
			}

			template<>
			void VertexLayout::Push<unsigned char>(unsigned int count)
			{
				m_Elements.push_back({ GL_UNSIGNED_BYTE, count, true });
				m_Stride += count * LayoutElement::GetSizeOfType(GL_UNSIGNED_BYTE);
			}
		}
	}
}