#include "agpch.h"
#include "OpenGLContext.h"

#include "Afterglow/Core/Assert.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace Afterglow
{
	OpenGLContext::OpenGLContext(void* windowHandle)
		: GraphicsContext(windowHandle)
	{
	}

	void OpenGLContext::Init()
	{
		glfwMakeContextCurrent(static_cast<GLFWwindow*>(m_WindowHandle));

		int gladInit = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		AG_ASSERT(gladInit, "Failed to initialize GLAD!");
	}

	void OpenGLContext::SwapBuffers()
	{
		glfwSwapBuffers(static_cast<GLFWwindow*>(m_WindowHandle));
	}
}