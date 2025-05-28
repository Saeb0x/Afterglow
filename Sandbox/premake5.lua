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
		"%{wks.location}/Afterglow/vendor/spdlog/include"
	}

	defines {
		"_CRT_SECURE_NO_WARNINGS"
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
		runtime "Debug"
		symbols "on"

		defines {
			"AG_DEVELOPMENT",
			"AG_ENABLE_ASSERTIONS"
		}

	filter "configurations:Testing"
		runtime "Release"
		optimize "on"

		defines "AG_TEST"

	filter "configurations:Shipping"
		runtime "Release"
		optimize "on"

		defines "AG_SHIPPING"