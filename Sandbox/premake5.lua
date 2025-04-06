project "Sandbox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("%{wks.location}/bin/" .. outputDir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputDir .. "/%{prj.name}")

	files {
		"src/**.h",
		"src/**.cpp"
	}

	includedirs {
		"%{wks.location}/Afterglow/src",
		"%{wks.location}/Afterglow/vendor"
	}

	links {
		"Afterglow"
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

	filter "configurations:Testing"
		defines "AG_TESTING"
		runtime "Release"
		optimize "on"

	filter "configurations:Shipping"
		defines "AG_SHIPPING"
		runtime "Release"
		optimize "on"