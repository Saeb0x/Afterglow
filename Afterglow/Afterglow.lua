project "Afterglow"
	kind "StaticLib"
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
		"src"
	}

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "On"

	filter "configurations:Release"
		runtime "Release"
		optimize "On"