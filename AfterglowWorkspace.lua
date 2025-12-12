workspace "AfterglowWorkspace"
	architecture "x86_64"
	configurations {
		"Debug",
		"Release"
	}
	startproject "Sandbox"

	OutputDirectory = "%{cfg.system}-%{cfg.buildcfg}-%{cfg.architecture}"
	VcpkgDirectory = "%{wks.location}/vcpkg_installed/x64-windows-static"

group "Core"
	include "Afterglow/Afterglow"
group ""

group "Playground"
	include "Sandbox/Sandbox"
group ""