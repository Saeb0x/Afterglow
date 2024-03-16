#include "Renderer.h"

#include <iostream>

namespace Afterglow {
	namespace Core {
		namespace Graphics
		{
			void GLClearError()
			{
				while (glGetError() != GL_NO_ERROR);
			}

			bool GLLogError(const char* function, const char* file, int line)
			{
				while (unsigned int error = glGetError())
				{
					std::cout << "[OpenGL Error: " << error << "] " << function << " " << file << " " << line << std::endl;
					return false;
				}

				return true;
			}

			Renderer& Renderer::GetInstance()
			{
				static Renderer rendererInstance;
				return rendererInstance;
			}

			void Renderer::Draw(const VertexArray& vao, const IndexBuffer& ibo, const Shader& shader)
			{
				vao.Bind();
				ibo.Bind();
				shader.Bind();

				GLCall(glDrawElements(GL_TRIANGLES, ibo.GetCount(), GL_UNSIGNED_INT, nullptr));
			}

			Renderer::Renderer() {}
			Renderer::~Renderer() {}
		}
	}
}

