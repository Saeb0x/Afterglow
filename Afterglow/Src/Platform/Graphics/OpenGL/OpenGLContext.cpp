#include "OpenGLContext.h"
#include "Core/Log.h"
#include <gl/GL.h>

// WGL extension for VSync control
typedef BOOL(WINAPI* PFNWGLSWAPINTERVALEXTPROC)(int interval);
PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = nullptr;

namespace Afterglow
{
	OpenGLContext::OpenGLContext(HWND windowHandle)
		: m_WindowHandle(windowHandle),
		m_DeviceContext(nullptr),
		m_RenderingContext(nullptr)
	{
	}

	OpenGLContext::~OpenGLContext()
	{
		if (m_RenderingContext)
		{
			wglMakeCurrent(nullptr, nullptr);
			wglDeleteContext(m_RenderingContext);
			m_RenderingContext = nullptr;
		}

		if (m_DeviceContext)
		{
			ReleaseDC(m_WindowHandle, m_DeviceContext);
			m_DeviceContext = nullptr;
		}
	}

	void OpenGLContext::Init()
	{
		m_DeviceContext = GetDC(m_WindowHandle);
		if (!m_DeviceContext)
		{
			AG_CORE_ERROR("Failed to get device context");
			return;
		}

		// Set pixel format
		PIXELFORMATDESCRIPTOR pfd = {};
		pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
		pfd.nVersion = 1;
		pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.cColorBits = 32;
		pfd.cDepthBits = 24;
		pfd.cStencilBits = 8;

		int pixelFormat = ChoosePixelFormat(m_DeviceContext, &pfd);
		if (pixelFormat == 0)
		{
			AG_CORE_ERROR("Failed to choose pixel format");
			return;
		}

		if (!SetPixelFormat(m_DeviceContext, pixelFormat, &pfd))
		{
			AG_CORE_ERROR("Failed to set pixel format");
			return;
		}

		m_RenderingContext = wglCreateContext(m_DeviceContext);
		if (!m_RenderingContext)
		{
			AG_CORE_ERROR("Failed to create OpenGL rendering context");
			return;
		}

		if (!wglMakeCurrent(m_DeviceContext, m_RenderingContext))
		{
			AG_CORE_ERROR("Failed to make OpenGL context current");
			return;
		}

		// Load VSync extension
		wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
		if (!wglSwapIntervalEXT)
		{
			AG_CORE_WARN("wglSwapIntervalEXT not supported - VSync control unavailable");
		}

		const char* vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
		const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
		const char* version = reinterpret_cast<const char*>(glGetString(GL_VERSION));

		AG_CORE_INFO("OpenGL Context Initialized");
		AG_CORE_INFO("	Vendor: {}", vendor ? vendor : "Unknown");
		AG_CORE_INFO("	Renderer: {}", renderer ? renderer : "Unknown");
		AG_CORE_INFO("	Version: {}", version ? version : "Unknown");
	}

	void OpenGLContext::SwapBuffers()
	{
		::SwapBuffers(m_DeviceContext);
	}

	void OpenGLContext::SetVSync(bool enabled)
	{
		if (wglSwapIntervalEXT)
		{
			wglSwapIntervalEXT(enabled ? 1 : 0);
			AG_CORE_INFO("VSync {}", enabled ? "enabled" : "disabled");
		}
		else
		{
			AG_CORE_WARN("Cannot set VSync - extension not available");
		}
	}
}