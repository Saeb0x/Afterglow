#include "agpch.h"
#include "Renderer2D.h"

#include "VertexArray.h"

namespace Afterglow
{
	struct Renderer2DStorage
	{
		std::shared_ptr<VertexArray> VertexArray;
	};
	static Renderer2DStorage s_Data;

	Renderer2D& Renderer2D::GetInstance()
	{
		static Renderer2D instance;
		return instance;
	}

	void Renderer2D::Init()
	{
		s_Data.VertexArray = VertexArray::Create();

		float squareVertices[4 * 5] =
		{
			// a_vertexPos			// a_texCoord
			-0.5f, -0.5f, 0.0f,		0.0f, 0.0f,
			 0.5f, -0.5f, 0.0f,		1.0f, 0.0f,
			 0.5f,  0.5f, 0.0f,		1.0f, 1.0f,
			-0.5f,  0.5f, 0.0f,		0.0f, 1.0f
		};

		std::shared_ptr<VertexBuffer> squareVB;
		squareVB = VertexBuffer::Create(sizeof(squareVertices), squareVertices);
		squareVB->SetLayout(
			{

				{ ShaderDataType::Float3, "a_vertexPos" },
				{ ShaderDataType::Float2, "a_texCoord" }
			}
		);
		s_Data.VertexArray->AddVertexBuffer(squareVB);

		uint32_t squareIndices[6] =
		{
			0, 1, 2,
			2, 3, 0
		};

		std::shared_ptr<IndexBuffer> squareIB;
		squareIB = IndexBuffer::Create(6, squareIndices);
		s_Data.VertexArray->SetIndexBuffer(squareIB);

		m_ShaderLibrary.Load("assets/shaders/FlatColor.glsl");
		m_ShaderLibrary.Load("assets/shaders/Texture.glsl");
		m_ShaderLibrary.Load("assets/shaders/Grid.glsl");
	}

	void Renderer2D::Shutdown()
	{
	}

	void Renderer2D::ResetStats()
	{
		m_Stats.DrawCalls = 0;
	}

	void Renderer2D::SetViewport(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
	{
		RenderCommand::SetViewport(x, y, width, height);
	}

	void Renderer2D::BeginScene(const Camera& camera)
	{
		auto flatColorShader = m_ShaderLibrary.Get("FlatColor");
		flatColorShader->Bind();
		flatColorShader->SetMat4("u_ProjectionViewMatrix", camera.GetProjectionViewMatrix());

		auto textureShader = m_ShaderLibrary.Get("Texture");
		textureShader->Bind();
		textureShader->SetMat4("u_ProjectionViewMatrix", camera.GetProjectionViewMatrix());
		textureShader->SetInt("u_Texture", 0);

		auto gridShader = m_ShaderLibrary.Get("Grid");
		gridShader->Bind();
		gridShader->SetMat4("u_ProjectionViewMatrix", camera.GetProjectionViewMatrix());
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
		auto flatColorShader = m_ShaderLibrary.Get("FlatColor");
		flatColorShader->Bind();

		glm::mat4 transform = glm::translate(glm::mat4(1.0), position) * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
		flatColorShader->SetMat4("u_ModelMatrix", transform);

		flatColorShader->SetFloat4("u_Color", color);

		RenderCommand::DrawIndexed(s_Data.VertexArray);
		m_Stats.DrawCalls++;
	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const std::shared_ptr<Texture2D>& texture)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, texture);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const std::shared_ptr<Texture2D>& texture)
	{
		auto textureShader = m_ShaderLibrary.Get("Texture");
		textureShader->Bind();

		glm::mat4 transform = glm::translate(glm::mat4(1.0), position) * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
		textureShader->SetMat4("u_ModelMatrix", transform);

		texture->Bind();
		RenderCommand::DrawIndexed(s_Data.VertexArray);
		m_Stats.DrawCalls++;
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec4& color)
	{
		DrawRotatedQuad({ position.x, position.y, 0.0f }, size, rotation, color);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const glm::vec4& color)
	{
		auto flatColorShader = m_ShaderLibrary.Get("FlatColor");
		flatColorShader->Bind();

		glm::mat4 transform = glm::translate(glm::mat4(1.0), position) * glm::rotate(glm::mat4(1.0f), glm::radians(rotation), { 0.0f, 0.0f, 1.0f }) * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
		flatColorShader->SetMat4("u_ModelMatrix", transform);

		flatColorShader->SetFloat4("u_Color", color);

		RenderCommand::DrawIndexed(s_Data.VertexArray);
		m_Stats.DrawCalls++;
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const std::shared_ptr<Texture2D>& texture)
	{
		DrawRotatedQuad({ position.x, position.y, 0.0f }, size, rotation, texture);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const std::shared_ptr<Texture2D>& texture)
	{
		auto textureShader = m_ShaderLibrary.Get("Texture");
		textureShader->Bind();

		glm::mat4 transform = glm::translate(glm::mat4(1.0), position) * glm::rotate(glm::mat4(1.0f), glm::radians(rotation), { 0.0f, 0.0f, 1.0f }) * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
		textureShader->SetMat4("u_ModelMatrix", transform);

		texture->Bind();
		RenderCommand::DrawIndexed(s_Data.VertexArray);
		m_Stats.DrawCalls++;
	}

	void Renderer2D::DrawGrid(float spacing, float thickness, const glm::vec3& gridColor, const glm::vec3& backgroundColor)
	{
		auto gridShader = m_ShaderLibrary.Get("Grid");
		gridShader->Bind();
		gridShader->SetFloat("u_Spacing", spacing);
		gridShader->SetFloat("u_Thickness", thickness);
		gridShader->SetFloat3("u_GridColor", gridColor);
		gridShader->SetFloat3("u_BGColor", backgroundColor);

		glm::mat4 transform = glm::scale(glm::mat4(1.0f), { 1000.0f, 1000.0f, 1.0f });
		gridShader->SetMat4("u_ModelMatrix", transform);

		RenderCommand::DrawIndexed(s_Data.VertexArray);
		m_Stats.DrawCalls++;
	}
}