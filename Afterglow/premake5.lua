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
		"src/**.cpp",
	}

	-- Exclude networking headers/sources by default.
	removefiles {
		"src/Afterglow/Networking/**.h",
		"src/Afterglow/Networking/**.cpp"
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

	if _OPTIONS["WithNetworking"] then
		files {
			"src/Afterglow/Networking/**.h",
			"src/Afterglow/Networking/**cpp"
		}

		includedirs {
			"%{includeDirs.curl}"
		}

		defines {
			"AG_NETWORKING",
			"CURL_STATICLIB"
		}

		links {
			"CURL"
		}
	end

	filter "system:windows"
		systemversion "latest"
		
		defines {
			"AG_PLATFORM_WINDOWS"
		}

	filter "configurations:Development"
		runtime "Debug"
		symbols "On"
		
		defines { 
			"AG_DEVELOPMENT"
		}

	filter "configurations:Test"
		runtime "Release"
		optimize "On"
		
		defines {
			"AG_TEST"
		}

	filter "configurations:Shipping"
		runtime "Release"
		symbols "Off"
		optimize "Full"
		
		defines { 
			"AG_SHIPPING"
		}