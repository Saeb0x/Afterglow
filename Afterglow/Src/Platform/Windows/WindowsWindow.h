#pragma once

#include "Core/Window.h"
#include <Windows.h>

namespace Afterglow
{
	class WindowsWindow : public Window
	{
	public:
		WindowsWindow(const WindowConfig& config);
		~WindowsWindow() override;

		void Update() override;
		bool ShouldClose() const override;

		uint32_t GetWidth() const override { return m_Config.Width; }
		uint32_t GetHeight() const override { return m_Config.Height; }

		void* GetNativeHandle() const override { return m_Handle; }

		void SetVSync(bool enabled) override;
		bool IsVSync() const override { return m_Config.VSync; };

	private:
		void Init();
		void Shutdown();

		static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	private:
		HWND m_Handle;
		HINSTANCE m_Instance;
		WindowConfig m_Config;
		bool b_ShouldClose;
	};
}