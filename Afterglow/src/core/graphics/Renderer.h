#pragma once

#include <GL/glew.h>

#include "VertexArray.h"
#include "IndexBuffer.h"
#include "Shader.h"

namespace Afterglow {
	namespace Core {
		namespace Graphics
		{
			#define ASSERT(x) if (!x) __debugbreak();
			#define GLCall(x) GLClearError();\
				x;\
				ASSERT(GLLogError(#x, __FILE__, __LINE__))

			void GLClearError();
			bool GLLogError(const char* function, const char* file, int line);

			class Renderer
			{
			public:
				static Renderer& GetInstance();
				void Draw(const VertexArray& vao, const IndexBuffer& ibo, const Shader& shader);
			private:
				Renderer();
				~Renderer();
				Renderer(const Renderer&);
				const Renderer& operator=(const Renderer&);
			};
		}
	}
}

