workspace "Afterglow"
	architecture "x86_64"
	startproject "Sandbox"

	configurations {
		"Development",
		"Test",
		"Shipping"
	}

outputDir = "%{cfg.system}-%{cfg.buildcfg}-%{cfg.architecture}"

-- Optional networking flag.
newoption {
	trigger = "WithNetworking",
	description = "Enable networking features in Afterglow."
}

group "Dependencies"
	include "Afterglow/vendor/Dependencies.lua"
group ""

group "Core"
	include "Afterglow"
group ""

group "Misc"
	include "Sandbox"
group ""