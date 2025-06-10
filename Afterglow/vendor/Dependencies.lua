includeDir = {}

includeDir["spdlog"] = "vendor/spdlog/include"
includeDir["glfw"] = "vendor/glfw/include"
includeDir["glad"] = "vendor/glad/include"
includeDir["imgui"] = "vendor/imgui"

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

project "GLAD"
	kind "StaticLib"
	location "glad"
	language "C"
	staticruntime "on"
	warnings "off"

	targetdir ("%{wks.location}/bin/" .. outputDir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputDir .. "/%{prj.name}")

	files {
		"glad/include/glad/glad.h",
		"glad/include/KHR/khrplatform.h",
		"glad/src/glad.c"
	}

	includedirs {
		"glad/include"
	}

	filter "system:windows"
		systemversion "latest"

		defines {
			"_CRT_SECURE_NO_WARNINGS"
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

project "IMGUI"
	kind "StaticLib"
	location "imgui"
	language "C++"
	staticruntime "on"
	warnings "off"

	targetdir ("%{wks.location}/bin/" .. outputDir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputDir .. "/%{prj.name}")

	files {
		"imgui/imconfig.h",
		"imgui/imgui.h",
		"imgui/imgui.cpp",
		"imgui/imgui_draw.cpp",
		"imgui/imgui_internal.h",
		"imgui/imgui_tables.cpp",
		"imgui/imgui_widgets.cpp",
		"imgui/imstb_rectpack.h",
		"imgui/imstb_textedit.h",
		"imgui/imstb_truetype.h",
		"imgui/imgui_demo.cpp",
		
		"imgui/backends/imgui_impl_glfw.h",
		"imgui/backends/imgui_impl_glfw.cpp",
		"imgui/backends/imgui_impl_opengl3.h",
		"imgui/backends/imgui_impl_opengl3.cpp"
	}

	includedirs {
		"imgui",
		"glfw/include"
	}

	links {
		"GLFW"
	}

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Development"
		runtime "Debug"
		symbols "on"

	filter "configurations:Test"
		runtime "Release"
		optimize "on"

	filter "configurations:Shipping"
		runtime "Release"
		optimize "on"