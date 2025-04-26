workspace "Afterglow"
	architecture "x86_64"
	startproject "Sandbox"

	configurations {
		"Development",
		"Testing",
		"Shipping"
	}

outputDir = "%{cfg.system}-%{cfg.buildcfg}-%{cfg.architecture}"

group "Dependencies"
	include "Afterglow/Vendor/Dependencies.lua"
group ""

group "Core"
	include "Afterglow"
group ""

group "Misc"
	include "Sandbox"
group ""