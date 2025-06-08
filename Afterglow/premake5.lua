project "Afterglow"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"

	-- Gives /MTd for Development and /MT for Test and Shipping.
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
		"%{includeDir.glfw}",
		"%{includeDir.glad}"
	}

	defines {
		"_CRT_SECURE_NO_WARNINGS",
		"GLFW_INCLUDE_NONE"
	}

	links {
		"opengl32.lib",
		"GLFW",
		"GLAD"
	}

	filter "system:windows"
		systemversion "latest"

		defines {
			"AG_PLATFORM_WINDOWS"
		}

	filter "configurations:Development"
		runtime "Debug"
		symbols "on"

		defines {
			"AG_DEVELOPMENT",
			"AG_ENABLE_ASSERTIONS"
		}

	filter "configurations:Test"
		runtime "Release"
		optimize "on"

		defines "AG_TEST"

	filter "configurations:Shipping"
		runtime "Release"
		optimize "on"

		defines "AG_SHIPPING"