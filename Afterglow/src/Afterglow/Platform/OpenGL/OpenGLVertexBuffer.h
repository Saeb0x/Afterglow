#pragma once

#include "Afterglow/Core/Renderer/Buffer.h"

namespace Afterglow
{
	class OpenGLVertexBuffer : public VertexBuffer
	{
	public:
		OpenGLVertexBuffer(uint32_t size, const float* vertices);
		~OpenGLVertexBuffer() override;

		void Bind() const override;
		void Unbind() const override;

		void SetLayout(const BufferLayout& layout) override { m_Layout = layout; }
		const BufferLayout& GetLayout() const override { return m_Layout; }

		uint32_t GetRendererID() const override { return m_RendererID; }

	private:
		uint32_t m_RendererID;
		BufferLayout m_Layout;
	};
}