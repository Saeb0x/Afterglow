#pragma once

#include "VertexBuffer.h"
#include "VertexLayout.h"

namespace Afterglow {
	namespace Core {
		namespace Graphics {
			class VertexArray
			{
			public:
				VertexArray();
				~VertexArray();

				void AddBuffer(const VertexBuffer& vbo, const VertexLayout& layout);

				void Bind() const;
				void Unbind() const;
			private:
				unsigned int m_RendererID;
			};
		}
	}
}



