#pragma once

#include <vector>

#define GL_FLOAT 0x1406
#define GL_UNSIGNED_INT 0x1405
#define GL_UNSIGNED_BYTE 0x1401

namespace Afterglow
{
	namespace Core
	{
		namespace Graphics
		{
			struct LayoutElement
			{
				unsigned int type;
				unsigned int count;
				bool normalize;

				static unsigned int GetSizeOfType(unsigned int type)
				{
					switch (type) {
						case GL_FLOAT: return 4;
						case GL_UNSIGNED_INT: return 4;
						case GL_UNSIGNED_BYTE: return 1;
					}

					return 0;
				}
			};

			class VertexLayout
			{
			public:
				VertexLayout();
				~VertexLayout();

				template<typename T>
				void Push(unsigned int count);

				template<>
				void Push<float>(unsigned int count);
				template<>
				void Push<unsigned int>(unsigned int count);
				template<>
				void Push<unsigned char>(unsigned int count);

				inline const std::vector<LayoutElement>& GetElements() const { return m_Elements; }
				inline unsigned int GetStride() const { return m_Stride; }
			private:
				std::vector<LayoutElement> m_Elements;
				unsigned int m_Stride;
			};
		}
	}
}

