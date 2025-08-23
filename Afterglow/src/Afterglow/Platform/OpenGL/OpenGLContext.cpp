#include "agpch.h"
#include "OpenGLContext.h"

#include "Afterglow/Core/Assert.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace Afterglow
{
	OpenGLContext::OpenGLContext(void* windowHandle)
		: m_WindowHandle(windowHandle)
	{
		AG_ASSERT(m_WindowHandle, "Window handle is null!");
	}

	void OpenGLContext::Init()
	{
		glfwMakeContextCurrent(static_cast<GLFWwindow*>(m_WindowHandle));

		int gladInit = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		AG_ASSERT(gladInit, "Failed to initialize GLAD!");

		AG_INFO("OpenGL context initialized:");
		AG_INFO("  Vendor: {0}", reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
		AG_INFO("  Renderer: {0}", reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
		AG_INFO("  Version: {0}", reinterpret_cast<const char*>(glGetString(GL_VERSION)));
	}

	void OpenGLContext::SwapBuffers()
	{
		glfwSwapBuffers(static_cast<GLFWwindow*>(m_WindowHandle));
	}
}