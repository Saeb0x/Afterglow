#pragma once

#include "Afterglow/Core/Renderer/Buffer.h"

namespace Afterglow
{
	class OpenGLIndexBuffer : public IndexBuffer
	{
	public:
		OpenGLIndexBuffer(uint32_t count, const uint32_t* indices);
		~OpenGLIndexBuffer() override;

		void Bind() const override;
		void Unbind() const override;

		uint32_t GetCount() const override { return m_Count; }
		uint32_t GetRendererID() const override { return m_RendererID; }

	private:
		uint32_t m_RendererID;
		uint32_t m_Count;
	};
}

