#include "WindowsWindow.h"
#include "Core/Log.h"

namespace Afterglow
{
	WindowsWindow::WindowsWindow(const WindowConfig& config)
		: m_Handle(nullptr),
		m_Instance(nullptr),
		m_Config(config),
		b_ShouldClose(false)
	{
		Init();
	}

	WindowsWindow::~WindowsWindow()
	{
		Shutdown();
	}

	void WindowsWindow::Init()
	{
		m_Instance = GetModuleHandle(nullptr);

		// Register window class
		WNDCLASSEX windowClass = {};
		windowClass.cbSize = sizeof(WNDCLASSEX);
		windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		windowClass.lpfnWndProc = WindowProc;
		windowClass.hInstance = m_Instance;
		windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
		windowClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		windowClass.lpszClassName = L"AfterglowWindowClass";
		
		if (!RegisterClassEx(&windowClass))
		{
			AG_CORE_ERROR("Failed to register window class");
			return;
		}
		
		// Calculate window size to get desired client area size
		DWORD style = WS_OVERLAPPEDWINDOW;
		if (!m_Config.Resizable)
		{
			style &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
		}

		RECT windowRect = { 0, 0, (LONG)m_Config.Width, (LONG)m_Config.Height };
		AdjustWindowRect(&windowRect, style, FALSE);

		int windowWidth = windowRect.right - windowRect.left;
		int windowHeight = windowRect.bottom - windowRect.top;

		// Center window on screen
		int screenWidth = GetSystemMetrics(SM_CXSCREEN);
		int screenHeight = GetSystemMetrics(SM_CYSCREEN);
		int x = (screenWidth - windowWidth) / 2;
		int y = (screenHeight - windowHeight) / 2;
		
		// Convert title to wide string
		int titleLength = MultiByteToWideChar(CP_UTF8, 0, m_Config.Title.c_str(), -1, nullptr, 0);
		wchar_t* wideTitle = new wchar_t[titleLength];
		MultiByteToWideChar(CP_UTF8, 0, m_Config.Title.c_str(), -1, wideTitle, titleLength);

		m_Handle = CreateWindowEx(
			0,
			L"AfterglowWindowClass",
			wideTitle,
			style,
			x, y,
			windowWidth, windowHeight,
			nullptr,
			nullptr,
			m_Instance,
			this	// To be retrieved in WindowProc
		);

		delete[] wideTitle;

		if (!m_Handle)
		{
			AG_CORE_ERROR("Failed to create window");
			return;
		}

		ShowWindow(m_Handle, SW_SHOW);
		UpdateWindow(m_Handle);

		AG_CORE_DEBUG("Window created: {} ({}x{})", m_Config.Title, m_Config.Width, m_Config.Height);
	}

	void WindowsWindow::Shutdown()
	{
		if (m_Handle)
		{
			DestroyWindow(m_Handle);
			m_Handle = nullptr;
		}

		UnregisterClass(L"AfterglowWindowClass", m_Instance);
		AG_CORE_DEBUG("Window destroyed");
	}

	void WindowsWindow::Update()
	{
		MSG msg;
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	bool WindowsWindow::ShouldClose() const
	{
		return b_ShouldClose;
	}

	void WindowsWindow::SetVSync(bool enabled)
	{
		if (m_Config.VSync != enabled)
		{
			m_Config.VSync = enabled;
			
			// TODO(saeb): Actually set VSync when we have OpenGL/DirectX context
			AG_CORE_DEBUG("VSync {}", enabled ? "enabled" : "disabled");
		}
	}

	LRESULT WindowsWindow::WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		WindowsWindow* window = nullptr;

		if (msg == WM_CREATE)
		{
			// Retrieve 'this' pointer from CreateWindowEx
			CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
			window = reinterpret_cast<WindowsWindow*>(pCreate->lpCreateParams);
			SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
		}
		else
		{
			// Retrieve 'this' pointer from window user data
			window = reinterpret_cast<WindowsWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
		}

		if (window)
		{
			switch (msg)
			{
				case WM_CLOSE:
				{
					window->b_ShouldClose = true;
					return 0;
				}
				case WM_DESTROY:
				{
					PostQuitMessage(0);
					return 0;
				}
				case WM_SIZE:
				{
					UINT width = LOWORD(lParam);
					UINT height = HIWORD(lParam);
					window->m_Config.Width = width;
					window->m_Config.Height = height;

					AG_CORE_DEBUG("Window resized: {}x{}", width, height);
					return 0;
				}
			}
		}

		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
}