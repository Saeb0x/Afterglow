workspace "AfterglowWorkspace"
	architecture "x86_64"
	configurations {
		"Debug",
		"Release"
	}
	startproject "Sandbox"

	OutputDirectory = "%{cfg.system}-%{cfg.buildcfg}-%{cfg.architecture}"

group "Core"
	include "Afterglow/Afterglow"
group ""

group "Playground"
	include "Sandbox/Sandbox"
group ""