#pragma once

namespace Afterglow {
	namespace Core {
		namespace Graphics {
			class IndexBuffer
			{
			public:
				IndexBuffer(const unsigned int* data, unsigned int count);
				~IndexBuffer();

				void Bind() const;
				void Unbind() const;
				inline int GetCount() const { return m_Count; }
			private:
				unsigned int m_RendererID;
				unsigned int m_Count;
			};
		}
	}
}

