project "Afterglow"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "On"
	
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
		"%{includeDirs.imgui}",
		"%{includeDirs.glm}"
	}

	defines {
		"GLFW_INCLUDE_NONE"
	}

	links {
		"GLFW",
		"GLAD",
		"IMGUI"
	}

	if _OPTIONS["WithNetworking"] then
		filter "system:windows"
			files {
				"src/Afterglow/Networking/**.h",
				"src/Afterglow/Networking/**cpp"
			}

			defines {
				"AG_NETWORKING",
				"CURL_STATICLIB"
			}

			includedirs {
				"%{includeDirs.libcurl}"
			}

			filter { "system:windows", "configurations:Development" }
				libdirs {
					"vendor/libcurl/lib/win64/debug"
				}

				links {
					"zlibd.lib",
					"libcurl-d.lib"
				}

			filter { "system:windows", "configurations:Test or Shipping" }
				libdirs {
					"vendor/libcurl/lib/win64/release"
				}

				links {
					"zlib.lib",
					"libcurl.lib"
				}
		filter {}
	end

	filter "system:windows"
		systemversion "latest"
		
		defines {
			"_CRT_SECURE_NO_WARNINGS",
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