#pragma once

#include "Afterglow/Renderer/Framebuffer.h"

namespace Afterglow
{
	class OpenGLFramebuffer : public Framebuffer
	{
	public:
		OpenGLFramebuffer(const FramebufferSpecification& spec);
		~OpenGLFramebuffer() override;

		void Resize(uint16_t width, uint16_t height) override;

		void Bind() const override;
		void Unbind() const override;

		const FramebufferSpecification& GetSpecification() const override { return m_Specification; }
		uint32_t GetColorAttachmentRendererID() const override { return m_ColorAttachment; }

	private:
		void Invalidate();

	private:
		uint32_t m_RendererID;
		uint32_t m_ColorAttachment, m_DepthAttachment;
		FramebufferSpecification m_Specification;
	};
}