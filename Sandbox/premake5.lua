project "Sandbox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "On"

	targetdir("../bin/" ..outputDir.. "/%{prj.name}")
	objdir("../bin-int/" ..outputDir.. "/%{prj.name}")

	files {
		"Src/**.h",
		"Src/**.cpp"
	}

	includedirs {
		"Src",
		"../Afterglow/Src",
		"../Afterglow/Vendor/spdlog/include"
	}

	links {
		"Afterglow"
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