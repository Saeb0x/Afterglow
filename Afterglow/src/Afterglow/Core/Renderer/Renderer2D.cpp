#include "agpch.h"
#include "Renderer2D.h"

#include "RenderCommand.h"
#include "VertexArray.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Afterglow
{
	struct Renderer2DStorage
	{
		std::shared_ptr<VertexArray> VertexArray;
	};
	static Renderer2DStorage* s_Data = new Renderer2DStorage();

	Renderer2D& Renderer2D::GetInstance()
	{
		static Renderer2D instance;
		return instance;
	}

	Renderer2D::~Renderer2D()
	{
		Shutdown();
	}

	void Renderer2D::Init()
	{
		s_Data->VertexArray.reset(VertexArray::Create());

		float squareVertices[4 * 5] =
		{
			// a_vertexPos			// a_texCoord
			-0.5f, -0.5f, 0.0f,		0.0f, 0.0f,
			 0.5f, -0.5f, 0.0f,		1.0f, 0.0f,
			 0.5f,  0.5f, 0.0f,		1.0f, 1.0f,
			-0.5f,  0.5f, 0.0f,		0.0f, 1.0f
		};

		std::shared_ptr<VertexBuffer> squareVB;
		squareVB.reset(VertexBuffer::Create(sizeof(squareVertices), squareVertices));
		squareVB->SetLayout(
			{

				{ ShaderDataType::Float3, "a_vertexPos" },
				{ ShaderDataType::Float2, "a_texCoord" }
			}
		);
		s_Data->VertexArray->AddVertexBuffer(squareVB);

		uint32_t squareIndices[6] =
		{
			0, 1, 2,
			2, 3, 0
		};

		std::shared_ptr<IndexBuffer> squareIB;
		squareIB.reset(IndexBuffer::Create(6, squareIndices));
		s_Data->VertexArray->SetIndexBuffer(squareIB);

		m_ShaderLibrary.Load("assets/shaders/FlatColor.glsl");
		m_ShaderLibrary.Load("assets/shaders/Texture.glsl");
	}

	void Renderer2D::Shutdown()
	{
		delete s_Data;
	}

	void Renderer2D::BeginScene(const OrthographicCamera& camera)
	{
		m_ShaderLibrary.Get("FlatColor")->Bind();
		m_ShaderLibrary.Get("FlatColor")->SetMat4("u_ProjectionViewMatrix", camera.GetProjectionViewMatrix());

		m_ShaderLibrary.Get("Texture")->Bind();
		m_ShaderLibrary.Get("Texture")->SetMat4("u_ProjectionViewMatrix", camera.GetProjectionViewMatrix());
		m_ShaderLibrary.Get("Texture")->SetInt("u_Texture", 0);
	}

	void Renderer2D::EndScene()
	{
	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, color);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
	{
		m_ShaderLibrary.Get("FlatColor")->Bind();

		glm::mat4 transform = glm::translate(glm::mat4(1.0), position) * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
		m_ShaderLibrary.Get("FlatColor")->SetMat4("u_ModelMatrix", transform);

		m_ShaderLibrary.Get("FlatColor")->SetFloat4("u_Color", color);

		RenderCommand::DrawIndexed(s_Data->VertexArray);
	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const std::shared_ptr<Texture2D>& texture)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, texture);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const std::shared_ptr<Texture2D>& texture)
	{
		m_ShaderLibrary.Get("Texture")->Bind();

		glm::mat4 transform = glm::translate(glm::mat4(1.0), position) * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
		m_ShaderLibrary.Get("Texture")->SetMat4("u_ModelMatrix", transform);

		texture->Bind();
		RenderCommand::DrawIndexed(s_Data->VertexArray);
	}
}