#include "VertexArray.h"

#include "Renderer.h"

namespace Afterglow {
	namespace Core {
		namespace Graphics {
			VertexArray::VertexArray()
			{
				GLCall(glGenVertexArrays(1, &m_RendererID));
				GLCall(glBindVertexArray(m_RendererID));
			}
			VertexArray::~VertexArray()
			{
				GLCall(glDeleteVertexArrays(1, &m_RendererID));
			}
			void VertexArray::AddBuffer(const VertexBuffer& vbo, const VertexLayout& layout)
			{
				vbo.Bind();

				const auto& elements = layout.GetElements();
				int offset = 0;
				for (unsigned int i = 0; i < elements.size(); i++)
				{
					const auto& element = elements[i];
					GLCall(glEnableVertexAttribArray(i));
					GLCall(glVertexAttribPointer(i, element.count, element.type, (element.normalize) ? GL_TRUE : GL_FALSE , layout.GetStride(), (const void*)offset));

					offset += element.count * LayoutElement::GetSizeOfType(element.type);
				}
			}
			void VertexArray::Bind() const
			{
				GLCall(glBindVertexArray(m_RendererID));
			}
			void VertexArray::Unbind() const
			{
				GLCall(glBindVertexArray(0));
			}
		}
	}
}


