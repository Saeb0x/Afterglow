includeDirs = {}

includeDirs["spdlog"] = "vendor/spdlog/include"
includeDirs["glfw"] = "vendor/glfw/include"
includeDirs["glad"] = "vendor/glad/include"
includeDirs["imgui"] = "vendor/imgui"
includeDirs["glm"] = "vendor/glm"
includeDirs["curl"] = "vendor/curl/include"

project "GLFW"
	location "glfw"
	kind "StaticLib"
	language "C"
	staticruntime "Off"
	warnings "Off"

	targetdir ("%{wks.location}/bin/" ..outputDir.. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" ..outputDir.. "/%{prj.name}")

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

	filter "system:macosx"
		pic "On"

		files {
			"glfw/src/cocoa_init.m",
			"glfw/src/cocoa_monitor.m",
			"glfw/src/cocoa_window.m",
			"glfw/src/cocoa_joystick.m", 
			"glfw/src/cocoa_time.c",
			"glfw/src/nsgl_context.m",
			"glfw/src/posix_thread.c",
			"glfw/src/posix_module.c",
			"glfw/src/osmesa_context.c",
			"glfw/src/egl_context.c"
		}

		defines "_GLFW_COCOA"

	filter "system:linux"
		systemversion "latest"
		pic "On"

		files {
			"glfw/src/x11_init.c",
			"glfw/src/x11_monitor.c",
			"glfw/src/x11_window.c",
			"glfw/src/linux_joystick.c",
			"glfw/src/xkb_unicode.c",
			"glfw/src/posix_module.c",
			"glfw/src/posix_time.c",
			"glfw/src/posix_thread.c",
			"glfw/src/posix_module.c",
			"glfw/src/glx_context.c",
			"glfw/src/egl_context.c",
			"glfw/src/osmesa_context.c"
		}

		defines "_GLFW_X11"

	filter "configurations:Development"
		runtime "Debug"
		symbols "On"

	filter "configurations:Test"
		runtime "Release"
		optimize "Speed"

	filter "configurations:Shipping"
		runtime "Release"
		symbols "Off"
		optimize "Speed"

project "GLAD"
	location "glad"
	kind "StaticLib"
	language "C"
	staticruntime "Off"
	warnings "Off"

	targetdir ("%{wks.location}/bin/" ..outputDir.. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" ..outputDir.. "/%{prj.name}")

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

	filter "configurations:Development"
		runtime "Debug"
		symbols "on"

	filter "configurations:Test"
		runtime "Release"
		optimize "Speed"

	filter "configurations:Shipping"
		runtime "Release"
		symbols "Off"
		optimize "Speed"

project "IMGUI"
	location "imgui"
	kind "StaticLib"
	language "C++"
	staticruntime "Off"
	warnings "Off"

	targetdir ("%{wks.location}/bin/" ..outputDir.. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" ..outputDir.. "/%{prj.name}")

	files {
		"imgui/imconfig.h",
		"imgui/imgui.h",
		"imgui/imgui.cpp",
		"imgui/imgui_demo.cpp",
		"imgui/imgui_draw.cpp",
		"imgui/imgui_internal.h",
		"imgui/imgui_tables.cpp",
		"imgui/imgui_widgets.cpp",
		"imgui/imstb_rectpack.h",
		"imgui/imstb_textedit.h",
		"imgui/imstb_truetype.h"
	}

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Development"
		runtime "Debug"
		symbols "On"

	filter "configurations:Test"
		runtime "Release"
		optimize "Speed"

	filter "configurations:Shipping"
		runtime "Release"
		symbols "Off"
		optimize "Speed"

if _OPTIONS["WithNetworking"] then 
	project "CURL"
		location "curl"
		kind "StaticLib"
		language "C"
		staticruntime "Off"
		warnings "Off"

		targetdir ("%{wks.location}/bin/" ..outputDir.. "/%{prj.name}")
		objdir ("%{wks.location}/bin-int/" ..outputDir.. "/%{prj.name}")

		files {
			"curl/lib/**.h",
			"curl/lib/**.c",
			"curl/src/**.h",
			"curl/src/**.c",
			"curl/include/**.h"
		}

		includedirs {
			"curl/include",
			"curl/lib"
		}

		defines {
			"BUILDING_LIBCURL",
			"CURL_STATICLIB",
			"USE_SCHANNEL",   
			"USE_WINDOWS_SSPI",
			"CURL_DISABLE_LDAP"
		}

		filter "system:windows"
			systemversion "latest"
        
			links {
				"ws2_32.lib",
				"crypt32.lib",
				"secur32.lib"
			}

		filter "configurations:Development"
			runtime "Debug"
			symbols "On"

		filter "configurations:Test"
			runtime "Release"
			optimize "Speed"

		filter "configurations:Shipping"
			runtime "Release"
			symbols "Off"
			optimize "Speed"
end