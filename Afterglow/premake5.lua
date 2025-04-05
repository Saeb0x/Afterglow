project "Afterglow"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"

	-- Gives /MTd for Development and /MT for Test and Ship.
	staticruntime "on"

	targetdir ("%{wks.location}/bin/" .. outputDir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputDir .. "/%{prj.name}")

	files {
		"src/**.h",
		"src/**.cpp"
	}

	includedirs {
		"src"
	}

	defines {
		"_CRT_SECURE_NO_WARNINGS"
	}

	links {
		"opengl32.lib"
	}

	filter "system:windows"
		systemversion "latest"

		defines {
			"AG_PLATFORM_WINDOWS"
		}

	filter "configurations:Development"
		defines "AG_DEVELOPMENT"
		runtime "Debug"
		symbols "on"

	filter "configurations:Test"
		defines "AG_TEST"
		runtime "Release"
		optimize "on"

	filter "configurations:Ship"
		defines "AG_SHIP"
		runtime "Release"
		optimize "on"