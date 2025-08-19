#include "agpch.h"
#include "WindowsInput.h"

#include "Afterglow/Core/Application.h"
#include "Afterglow/Core/Assert.h"

#include <GLFW/glfw3.h>

namespace Afterglow
{
#ifdef AG_PLATFORM_WINDOWS
	static WindowsInput s_WindowInput;
	Input* Input::s_Instance = &s_WindowInput;
#else
	AG_ASSERT(false, "Afterglow only supports Windows for now.");
	Input* Input::s_Instance = nullptr;
#endif

	bool WindowsInput::IsKeyPressedImpl(int keyCode) const
	{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());

		auto state = glfwGetKey(window, keyCode);
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	bool WindowsInput::IsMouseButtonPressedImpl(int mouseButton) const
	{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());

		auto state = glfwGetMouseButton(window, mouseButton);
		return state == GLFW_PRESS;
	}

	std::pair<float, float> WindowsInput::GetMousePositionImpl() const
	{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());

		double xPos, yPos;
		glfwGetCursorPos(window, &xPos, &yPos);

		return { (float)xPos, (float)yPos };
	}

	float WindowsInput::GetMouseXImpl() const
	{
		auto [x, y] = GetMousePositionImpl();
		return x;
	}

	float WindowsInput::GetMouseYImpl() const
	{
		auto [x, y] = GetMousePositionImpl();
		return y;
	}
}
