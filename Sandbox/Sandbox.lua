project "Sandbox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "On"

	targetdir("%{wks.location}/out/%{prj.name}/" ..OutputDirectory)
	objdir("%{wks.location}/inter/%{prj.name}/" ..OutputDirectory)

	files {
		"src/**.h",
		"src/**.cpp"
	}

	includedirs {
		"src",
		"%{wks.location}/Afterglow/src",
		VcpkgDirectory.. "/include"
	}

	links {
		"Afterglow"
	}

	filter "system:windows"
		systemversion "latest"

		defines {
			"AG_PLATFORM_WINDOWS"
		}

		buildoptions {
			"/utf-8"
		}

	filter "configurations:Debug"
		runtime "Debug"
		symbols "On"

		defines {
			"AG_DEBUG"
		}

		libdirs {
			VcpkgDirectory.. "/debug/lib"
		}

		links {
			"fmtd"
		}

	filter "configurations:Release"
		runtime "Release"
		optimize "On"

		defines {
			"AG_RELEASE"
		}

		libdirs {
			VcpkgDirectory.. "/lib"
		}

		links {
			"fmt"
		}