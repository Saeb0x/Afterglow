project "Afterglow"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "On"

	targetdir("../bin/" ..outputDir.. "/%{prj.name}")
	objdir("../bin-int/" ..outputDir.. "/%{prj.name}")

	pchheader "agpch.h"
	pchsource "Src/agpch.cpp"

	include "Vendor/Dependencies.lua"

	files {
		"Src/**.h",
		"Src/**.cpp"
	}

	includedirs {
		"Src",
		"%{includeDirs.spdlog}"
	}

	links {
		"opengl32.lib"
	}

	defines {
		"_CRT_SECURE_NO_WARNINGS",
		"AG_PLATFORMS_WINDOWS"
	}

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "On"

	filter "configurations:Release"
		runtime "Release"
		optimize "On"