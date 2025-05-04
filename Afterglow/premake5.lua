project "Afterglow"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"

	-- Gives /MTd for Development and /MT for Testing and Shipping.
	staticruntime "on"

	targetdir ("%{wks.location}/bin/" .. outputDir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputDir .. "/%{prj.name}")

	pchheader "agpch.h"
	pchsource "src/agpch.cpp"

	files {
		"src/**.h",
		"src/**.cpp"
	}

	includedirs {
		"src",
		"%{includeDir.spdlog}",
		"%{includeDir.glfw}"
	}

	defines {
		"_CRT_SECURE_NO_WARNINGS"
	}

	links {
		"opengl32.lib",
		"GLFW"
	}

	filter "system:windows"
		systemversion "latest"

		defines {
			"AG_PLATFORM_WINDOWS"
		}

	filter "configurations:Development"
		defines {
			"AG_DEVELOPMENT",
			"AG_ENABLE_ASSERTIONS"
		}
		runtime "Debug"
		symbols "on"

	filter "configurations:Testing"
		defines "AG_TESTING"
		runtime "Release"
		optimize "on"

	filter "configurations:Shipping"
		defines "AG_SHIPPING"
		runtime "Release"
		optimize "on"