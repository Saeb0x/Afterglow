#include "agpch.h"
#include "OpenGLFramebuffer.h"

#include "Afterglow/Core/Assert.h"

#include <glad/glad.h>

namespace Afterglow
{
    OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferSpecification& spec)
        : m_Specification(spec)
    {
        Invalidate();
    }

    OpenGLFramebuffer::~OpenGLFramebuffer()
    {
        glDeleteFramebuffers(1, &m_RendererID);
        glDeleteTextures(1, &m_ColorAttachment);
        glDeleteTextures(1, &m_DepthAttachment);
    }

    void OpenGLFramebuffer::Resize(uint16_t width, uint16_t height)
    {
        m_Specification.Width = width; m_Specification.Height = height;
        Invalidate();
    }

    void OpenGLFramebuffer::Invalidate()
    {
        if (m_RendererID)
        {
            glDeleteFramebuffers(1, &m_RendererID);
            glDeleteTextures(1, &m_ColorAttachment);
            glDeleteTextures(1, &m_DepthAttachment);
        }

        glCreateFramebuffers(1, &m_RendererID);
        glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

        // Color Attachment.
        glCreateTextures(GL_TEXTURE_2D, 1, &m_ColorAttachment);
        glTextureStorage2D(m_ColorAttachment, 1, GL_RGBA8, m_Specification.Width, m_Specification.Height);

        glTextureParameteri(m_ColorAttachment, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(m_ColorAttachment, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(m_ColorAttachment, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(m_ColorAttachment, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ColorAttachment, 0);

        // Depth/Stencil Attachment.
        glCreateTextures(GL_TEXTURE_2D, 1, &m_DepthAttachment);
        glTextureStorage2D(m_DepthAttachment, 1, GL_DEPTH24_STENCIL8, m_Specification.Width, m_Specification.Height);

        glTextureParameteri(m_DepthAttachment, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(m_DepthAttachment, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_DepthAttachment, 0);

        AG_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete!");

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void OpenGLFramebuffer::Bind() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
        glViewport(0, 0, m_Specification.Width, m_Specification.Height);
    }

    void OpenGLFramebuffer::Unbind() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}