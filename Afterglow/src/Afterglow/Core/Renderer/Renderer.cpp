#include "agpch.h"
#include "Renderer.h"

#include "RenderCommand.h"

namespace Afterglow
{
	void Renderer::BeginScene(OrthographicCamera& camera)
	{
		s_SceneData->ProjectionViewMatrix = camera.GetProjectionViewMatrix();
	}

	void Renderer::EndScene()
	{
	}

	void Renderer::Submit(const std::shared_ptr<Shader>& shader, const std::shared_ptr<VertexArray>& vertexArray)
	{
		shader->Bind();
		shader->UniformMat4("u_ProjectionViewMatrix", s_SceneData->ProjectionViewMatrix);

		RenderCommand::DrawIndexed(vertexArray);
	}
}