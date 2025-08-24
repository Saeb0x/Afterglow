#include "agpch.h"

#include "Afterglow/Core/PlatformUtils.h"

#include <GLFW/glfw3.h>

namespace Afterglow
{
	float Time::GetTime()
	{
		return (float)glfwGetTime();
	}
}