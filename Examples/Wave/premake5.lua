project "Wave"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "Off"
    multiprocessorcompile "On"

    targetdir "../../Build/%{prj.name}/%{cfg.system}-%{cfg.buildcfg}-%{cfg.architecture}/Bin"
    objdir "../../Build/%{prj.name}/%{cfg.system}-%{cfg.buildcfg}-%{cfg.architecture}/Bin-Int"

    files {
        "Src/**.h",
        "Src/**.cpp"
    }

    includedirs {
        "Src"
    }

    externalincludedirs {
        "../../Afterglow/Src",
        "../../%{IncludeDir.vcpkg}"
    }
    externalwarnings "Off"

    links {
        "Afterglow"
    }

    filter "system:windows"
        systemversion "latest"

        defines {
            "AG_PLATFORM_WINDOWS"
        }

        links {
            "opengl32",
            "glad"
        }

    filter "configurations:Debug"
        runtime "Debug"
        symbols "On"
        targetsuffix "-d"

        defines {
            "AG_DEBUG"
        }

        libdirs {
            "../../%{LibsDir.vcpkg_debug}"
        }

        links {}
        
    filter "configurations:Release"
        runtime "Release"
        optimize "On"

        defines {
            "AG_RELEASE"
        }

        libdirs {
            "../../%{LibsDir.vcpkg_release}"
        }

        links {}