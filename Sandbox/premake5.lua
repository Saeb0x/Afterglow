project "Sandbox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "On"

	targetdir ("%{wks.location}/bin/" ..outputDir.. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" ..outputDir.. "/%{prj.name}")

	files {
		"src/**.h",
		"src/**.cpp"
	}

	includedirs {
		"src",
		"%{wks.location}/Afterglow/src",
		"%{wks.location}/Afterglow/%{includeDirs.spdlog}",
		"%{wks.location}/Afterglow/%{includeDirs.imgui}",
		"%{wks.location}/Afterglow/%{includeDirs.glm}"
	}

	links {
		"Afterglow"
	}

	if _OPTIONS["WithNetworking"] then
		filter "system:windows"
			defines {
				"AG_NETWORKING",
				"CURL_STATICLIB"
			}

			includedirs {
				"%{wks.location}/Afterglow/%{includeDirs.libcurl}"
			}
		filter {}
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

	filter "configurations:Testing"
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