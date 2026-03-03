#pragma once

#include "Core/Window.h"
#include <Windows.h>
#include <memory>

namespace Afterglow
{
	enum class Key;
	class GraphicsContext;

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

		GraphicsContext& GetContext() override { return *m_Context; }

	private:
		void Init();
		void Shutdown();

		static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
		
		static Key TranslateWin32KeyCode(WPARAM wParam, LPARAM lParam);

	private:
		HWND m_Handle;
		HINSTANCE m_Instance;
		WindowConfig m_Config;
		bool b_ShouldClose;
		std::unique_ptr<GraphicsContext> m_Context;
	};
}