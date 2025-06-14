#pragma once

namespace Afterglow
{
	class Input
	{
	public:
		inline static bool IsKeyPressed(int keyCode) { return s_Instance->IsKeyPressedImpl(keyCode); }
		inline static bool IsMouseButtonPressed(int mouseButton) { return s_Instance->IsMouseButtonPressedImpl(mouseButton); }
		inline static std::pair<float, float> GetMousePosition() { return s_Instance->GetMousePositionImpl(); }
		inline static float GetMouseX() { return s_Instance->GetMouseXImpl(); };
		inline static float GetMouseY() { return s_Instance->GetMouseYImpl(); };

	protected:
		virtual bool IsKeyPressedImpl(int keyCode) const = 0;
		virtual bool IsMouseButtonPressedImpl(int mouseButton) const = 0;
		virtual std::pair<float, float> GetMousePositionImpl() const = 0;
		virtual float GetMouseXImpl() const = 0;
		virtual float GetMouseYImpl() const = 0;

	private:
		static Input* s_Instance;
	};
}
