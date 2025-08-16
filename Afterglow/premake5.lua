project "Afterglow"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "Off"
	
	targetdir ("%{wks.location}/bin/" ..outputDir.. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" ..outputDir.. "/%{prj.name}")

	pchheader "agpch.h"
	pchsource "src/agpch.cpp"

	files {
		"src/**.h",
		"src/**.cpp"
	}

	includedirs {
		"src",
		"%{includeDirs.spdlog}",
		"%{includeDirs.glfw}",
		"%{includeDirs.glad}",
		"%{includeDirs.imgui}"
	}

	defines {
		"_CRT_SECURE_NO_WARNINGS",
		"GLFW_INCLUDE_NONE"
	}

	links {
		"GLFW",
		"GLAD",
		"IMGUI"
	}

	filter "system:windows"
		systemversion "latest"
		defines "AG_PLATFORM_WINDOWS"

	filter "configurations:Development"
		runtime "Debug"
		symbols "On"
		defines "AG_DEVELOPMENT"

	filter "configurations:Test"
		runtime "Release"
		optimize "On"
		defines "AG_TEST"

	filter "configurations:Shipping"
		runtime "Release"
		optimize "On"
		defines "AG_SHIPPING"