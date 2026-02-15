project "Sandbox"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "Off"

    targetdir "../../Build/%{prj.name}/%{cfg.system}-%{cfg.buildcfg}-%{cfg.architecture}/Bin"
    objdir "../../Build/%{prj.name}/%{cfg.system}-%{cfg.buildcfg}-%{cfg.architecture}/Bin-Int"

    files {
        "Src/**.h",
        "Src/**.cpp"
    }

    includedirs {
        "../../Afterglow/Src",
        "Src"
    }

    links {
        "Afterglow"
    }

    filter "system:windows"
        systemversion "latest"
        defines "AG_PLATFORM_WINDOWS"

    filter "configurations:Debug"
        defines "AG_DEBUG"
        runtime "Debug"
        symbols "On"
        targetsuffix "-d"
        
    filter "configurations:Release"
        defines "AG_RELEASE"
        runtime "Release"
        optimize "On"