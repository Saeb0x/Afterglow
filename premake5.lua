workspace "Afterglow"
	architecture "x86_64"

	configurations {
		"Debug",
		"Release"
	}

	-- Supported only by Visual Studio ------
	startproject "Sandbox"
	-----------------------------------------

outputDir = "%{cfg.system}-%{cfg.architecture}-%{cfg.buildcfg}"

include "Afterglow"
include "Sandbox"

if _ACTION == "clean" then
	os.rmdir("bin")
	os.rmdir("bin-int")

	os.remove("Afterglow.sln")
	
	os.remove("Sandbox/Sandbox.vcxproj")
	os.remove("Sandbox/Sandbox.vcxproj.filters")

	os.remove("Afterglow/Afterglow.vcxproj")
	os.remove("Afterglow/Afterglow.vcxproj.filters")
end