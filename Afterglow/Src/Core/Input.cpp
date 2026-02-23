#include "Input.h"
#include <cstring>

namespace Afterglow
{
	bool Input::s_KeyStates[512] = { false };
	bool Input::s_PrevKeyStates[512] = { false };

	bool Input::s_MouseButtonStates[8] = { false };
	bool Input::s_PrevMouseButtonStates[8] = { false };

	float Input::s_MouseX = 0.0f;
	float Input::s_MouseY = 0.0f;

	bool Input::IsKeyPressed(Key key)
	{
		int keyCode = static_cast<int>(key);
		return s_KeyStates[keyCode] && !s_PrevKeyStates[keyCode];
	}

	bool Input::IsKeyDown(Key key)
	{
		int keyCode = static_cast<int>(key);
		return s_KeyStates[keyCode];
	}

	bool Input::IsKeyReleased(Key key)
	{
		int keyCode = static_cast<int>(key);
		return !s_KeyStates[keyCode] && s_PrevKeyStates[keyCode];
	}

	bool Input::IsMouseButtonPressed(MouseButton button)
	{
		int buttonCode = static_cast<int>(button);
		return s_MouseButtonStates[buttonCode] && !s_PrevMouseButtonStates[buttonCode];
	}

	bool Input::IsMouseButtonDown(MouseButton button)
	{
		int buttonCode = static_cast<int>(button);
		return s_MouseButtonStates[buttonCode];
	}

	bool Input::IsMouseButtonReleased(MouseButton button)
	{
		int buttonCode = static_cast<int>(button);
		return !s_MouseButtonStates[buttonCode] && s_PrevMouseButtonStates[buttonCode];
	}

	std::pair<float, float> Input::GetMousePosition()
	{
		return { s_MouseX, s_MouseY };
	}

	float Input::GetMouseX()
	{
		return s_MouseX;
	}

	float Input::GetMouseY()
	{
		return s_MouseY;
	}

	void Input::SetKeyState(Key key, bool pressed)
	{
		int keyCode = static_cast<int>(key);
		s_KeyStates[keyCode] = pressed;
	}

	void Input::SetMouseButtonState(MouseButton button, bool pressed)
	{
		int buttonCode = static_cast<int>(button);
		s_MouseButtonStates[buttonCode] = pressed;
	}

	void Input::SetMousePosition(float x, float y)
	{
		s_MouseX = x;
		s_MouseY = y;
	}

	void Input::Update()
	{
		// Copy current state to previous state
		std::memcpy(s_PrevKeyStates, s_KeyStates, sizeof(s_KeyStates));
		std::memcpy(s_PrevMouseButtonStates, s_MouseButtonStates, sizeof(s_MouseButtonStates));
	}
}