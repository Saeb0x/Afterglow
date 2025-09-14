#pragma once

namespace Afterglow
{
	struct FramebufferSpecification
	{
		uint16_t Width, Height;
	};

	class Framebuffer
	{
	public:
		virtual ~Framebuffer() = default;

		virtual void Resize(uint16_t width, uint16_t height) = 0;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual const FramebufferSpecification& GetSpecification() const = 0;
		virtual uint32_t GetColorAttachmentRendererID() const = 0;

		static std::shared_ptr<Framebuffer> Create(const FramebufferSpecification& spec);
	};
}