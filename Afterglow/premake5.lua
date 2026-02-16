project "Afterglow"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "Off"
    
    targetdir "../Build/%{prj.name}/%{cfg.system}-%{cfg.buildcfg}-%{cfg.architecture}/Bin"
    objdir "../Build/%{prj.name}/%{cfg.system}-%{cfg.buildcfg}-%{cfg.architecture}/Bin-Int"

    files {
        "Src/**.h",
        "Src/**.cpp"
    }

    includedirs {
        "Src",
        "../%{IncludeDir.vcpkg}"
    }

    filter "system:windows"
        systemversion "latest"
        defines "AG_PLATFORM_WINDOWS"

    filter "configurations:Debug"
        defines "AG_DEBUG"
        runtime "Debug"
        symbols "On"
        targetsuffix "-d"
        
        libdirs "../%{LibsDir.vcpkg_debug}"
        links {}

    filter "configurations:Release"
        defines "AG_RELEASE"
        runtime "Release"
        optimize "On"

        libdirs "../%{LibsDir.vcpkg_release}"
        links {}