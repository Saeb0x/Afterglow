#include "WindowsWindow.h"
#include "Core/Log.h"
#include "Core/Input.h"
#include <windowsx.h>

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
				case WM_KEYDOWN:
				case WM_SYSKEYDOWN:
				{
					Key key = TranslateWin32KeyCode(wParam, lParam);
					Input::SetKeyState(key, true);
					
					return 0;
				}
				case WM_KEYUP:
				case WM_SYSKEYUP:
				{
					Key key = TranslateWin32KeyCode(wParam, lParam);
					Input::SetKeyState(key, false);

					return 0;
				}
				case WM_LBUTTONDOWN:
				{
					Input::SetMouseButtonState(MouseButton::Left, true);
					
					return 0;
				}
				case WM_LBUTTONUP:
				{
					Input::SetMouseButtonState(MouseButton::Left, false);
					
					return 0;
				}
				case WM_RBUTTONDOWN:
				{
					Input::SetMouseButtonState(MouseButton::Right, true);
					
					return 0;
				}
				case WM_RBUTTONUP:
				{
					Input::SetMouseButtonState(MouseButton::Right, false);

					return 0;
				}
				case WM_MBUTTONDOWN:
				{
					Input::SetMouseButtonState(MouseButton::Middle, true);
					
					return 0;
				}
				case WM_MBUTTONUP:
				{
					Input::SetMouseButtonState(MouseButton::Middle, false);
					
					return 0;
				}
				case WM_MOUSEMOVE:
				{
					float x = static_cast<float>(GET_X_LPARAM(lParam));
					float y = static_cast<float>(GET_Y_LPARAM(lParam));
					Input::SetMousePosition(x, y);

					return 0;
				}
			}
		}

		return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	Key WindowsWindow::TranslateWin32KeyCode(WPARAM wParam, LPARAM lParam)
	{
		switch (wParam)
		{
			// Alphabet
			case 'A': return Key::A;
			case 'B': return Key::B;
			case 'C': return Key::C;
			case 'D': return Key::D;
			case 'E': return Key::E;
			case 'F': return Key::F;
			case 'G': return Key::G;
			case 'H': return Key::H;
			case 'I': return Key::I;
			case 'J': return Key::J;
			case 'K': return Key::K;
			case 'L': return Key::L;
			case 'M': return Key::M;
			case 'N': return Key::N;
			case 'O': return Key::O;
			case 'P': return Key::P;
			case 'Q': return Key::Q;
			case 'R': return Key::R;
			case 'S': return Key::S;
			case 'T': return Key::T;
			case 'U': return Key::U;
			case 'V': return Key::V;
			case 'W': return Key::W;
			case 'X': return Key::X;
			case 'Y': return Key::Y;
			case 'Z': return Key::Z;

			// Numbers
			case '0': return Key::D0;
			case '1': return Key::D1;
			case '2': return Key::D2;
			case '3': return Key::D3;
			case '4': return Key::D4;
			case '5': return Key::D5;
			case '6': return Key::D6;
			case '7': return Key::D7;
			case '8': return Key::D8;
			case '9': return Key::D9;

			// Function keys
			case VK_F1: return Key::F1;
			case VK_F2: return Key::F2;
			case VK_F3: return Key::F3;
			case VK_F4: return Key::F4;
			case VK_F5: return Key::F5;
			case VK_F6: return Key::F6;
			case VK_F7: return Key::F7;
			case VK_F8: return Key::F8;
			case VK_F9: return Key::F9;
			case VK_F10: return Key::F10;
			case VK_F11: return Key::F11;
			case VK_F12: return Key::F12;

			// Special keys
			case VK_SPACE: return Key::Space;
			case VK_ESCAPE: return Key::Escape;
			case VK_RETURN: return Key::Enter;
			case VK_TAB: return Key::Tab;
			case VK_BACK: return Key::Backspace;
			case VK_INSERT: return Key::Insert;
			case VK_DELETE: return Key::Delete;
			case VK_RIGHT: return Key::Right;
			case VK_LEFT: return Key::Left;
			case VK_DOWN: return Key::Down;
			case VK_UP: return Key::Up;
			case VK_PRIOR: return Key::PageUp;
			case VK_NEXT: return Key::PageDown;
			case VK_HOME: return Key::Home;
			case VK_END: return Key::End;

			// Modifiers
			case VK_LSHIFT: return Key::LeftShift;
			case VK_RSHIFT: return Key::RightShift;
			case VK_LCONTROL: return Key::LeftControl;
			case VK_RCONTROL: return Key::RightControl;
			case VK_LMENU: return Key::LeftAlt;
			case VK_RMENU: return Key::RightAlt;

			// Punctuation
			case VK_OEM_1: return Key::Semicolon;
			case VK_OEM_PLUS: return Key::Equal;
			case VK_OEM_COMMA: return Key::Comma;
			case VK_OEM_MINUS: return Key::Minus;
			case VK_OEM_PERIOD: return Key::Period;
			case VK_OEM_2: return Key::Slash;
			case VK_OEM_3: return Key::GraveAccent;
			case VK_OEM_4: return Key::LeftBracket;
			case VK_OEM_5: return Key::Backslash;
			case VK_OEM_6: return Key::RightBracket;
			case VK_OEM_7: return Key::Apostrophe;

			// Numpad
			case VK_NUMPAD0: return Key::KP0;
			case VK_NUMPAD1: return Key::KP1;
			case VK_NUMPAD2: return Key::KP2;
			case VK_NUMPAD3: return Key::KP3;
			case VK_NUMPAD4: return Key::KP4;
			case VK_NUMPAD5: return Key::KP5;
			case VK_NUMPAD6: return Key::KP6;
			case VK_NUMPAD7: return Key::KP7;
			case VK_NUMPAD8: return Key::KP8;
			case VK_NUMPAD9: return Key::KP9;
			case VK_DECIMAL: return Key::KPDecimal;
			case VK_DIVIDE: return Key::KPDivide;
			case VK_MULTIPLY: return Key::KPMultiply;
			case VK_SUBTRACT: return Key::KPSubtract;
			case VK_ADD: return Key::KPAdd;

			default: return Key::Space;
		}
	}
}