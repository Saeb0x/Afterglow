workspace "Afterglow"
	architecture "x86_64"
	startproject "Sandbox"

	configurations {
		"Development",
		"Test",
		"Ship"
	}

outputDir = "%{cfg.system}-%{cfg.buildcfg}-%{cfg.architecture}"

group "Dependencies"

group ""

group "Core"
	include "Afterglow"
group ""

group "Misc"
	include "Sandbox"
group ""