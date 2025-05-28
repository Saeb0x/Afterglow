includeDir = {}

includeDir["spdlog"] = "vendor/spdlog/include"
includeDir["glfw"] = "vendor/glfw/include"

project "GLFW"
	kind "StaticLib"
	location "glfw"
	language "C"
	staticruntime "on"
	warnings "off"

	targetdir ("%{wks.location}/bin/" .. outputDir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputDir .. "/%{prj.name}")

	files {
		"glfw/include/GLFW/glfw3.h",
		"glfw/include/GLFW/glfw3native.h",

		"glfw/src/context.c",
		"glfw/src/init.c",
		"glfw/src/input.c",
		"glfw/src/monitor.c",

		"glfw/src/null_init.c",
		"glfw/src/null_joystick.c",
		"glfw/src/null_monitor.c",
		"glfw/src/null_window.c",

		"glfw/src/platform.c",
		"glfw/src/vulkan.c",
		"glfw/src/window.c"
	}

	filter "system:windows"
		systemversion "latest"

		files {
			"glfw/src/win32_init.c",
			"glfw/src/win32_joystick.c",
			"glfw/src/win32_module.c",
			"glfw/src/win32_monitor.c",
			"glfw/src/win32_time.c",
			"glfw/src/win32_thread.c",
			"glfw/src/win32_window.c",
			"glfw/src/wgl_context.c",
			"glfw/src/egl_context.c",
			"glfw/src/osmesa_context.c"
		}

		defines {
			"_CRT_SECURE_NO_WARNINGS",
			"_GLFW_WIN32"
		}

	filter "configurations:Development"
		runtime "Debug"
		symbols "on"

	filter "configurations:Test"
		runtime "Release"
		optimize "on"

	filter "configurations:Shipping"
		runtime "Release"
		optimize "on"