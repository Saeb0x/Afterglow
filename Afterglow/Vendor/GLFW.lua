project "GLFW"
	kind "StaticLib"
	language "C"
	staticruntime "On"
	warnings "Off"

	targetdir("../../bin/" ..outputDir.. "/Dependencies/%{prj.name}")
	objdir("../../bin-int/" ..outputDir.. "/Dependencies/%{prj.name}")

	defines {
		"_CRT_SECURE_NO_WARNINGS"
	}

	files {
		"glfw/include/**.h",
		"glfw/src/internal.h",
		"glfw/src/mappings.h",
		"glfw/src/init.c",
		"glfw/src/platform.h",
		"glfw/src/platform.c",
		"glfw/src/context.c",
		"glfw/src/monitor.c",
		"glfw/src/window.c",
		"glfw/src/input.c",
		"glfw/src/null_init.c",
		"glfw/src/null_joystick.h",
		"glfw/src/null_joystick.c",
		"glfw/src/null_monitor.c",
		"glfw/src/null_platform.h",
		"glfw/src/null_window.c",
		"glfw/src/vulkan.c"
	}
	
	filter "system:windows"
		systemversion "latest"

		defines {
			"_GLFW_WIN32"
		}
		
		files {
			"glfw/src/win32_init.c",
			"glfw/src/win32_window.c",
			"glfw/src/win32_time.h",
			"glfw/src/win32_time.c",
			"glfw/src/win32_thread.h",
			"glfw/src/win32_thread.c",
			"glfw/src/win32_platform.h",
			"glfw/src/win32_monitor.c",
			"glfw/src/win32_module.c",
			"glfw/src/win32_joystick.h",
			"glfw/src/win32_joystick.c",
			"glfw/src/wgl_context.c",
			"glfw/src/egl_context.c",
			"glfw/src/osmesa_context.c"
		}

	filter "configurations:Debug"
		runtime "Debug"
		symbols "On"
	
	filter "configurations:Release"
		runtime "Release"
		optimize "On"