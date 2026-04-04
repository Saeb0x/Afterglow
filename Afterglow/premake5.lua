project "Afterglow"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "Off"
    multiprocessorcompile "On"

    targetdir "../Build/%{prj.name}/%{cfg.system}-%{cfg.buildcfg}-%{cfg.architecture}/Bin"
    objdir "../Build/%{prj.name}/%{cfg.system}-%{cfg.buildcfg}-%{cfg.architecture}/Bin-Int"

    files {
        "Src/**.h",
        "Src/**.cpp"
    }

    includedirs {
        "Src"
    }

    externalincludedirs {
        "../%{IncludeDir.vcpkg}"
    }
    externalwarnings "Off"

    -- Remove all platform-specific files by default.
    removefiles {
        "Src/Platform/**"
    }

    filter "system:windows"
        systemversion "latest"

        defines {
            "AG_PLATFORM_WINDOWS"
        }

        -- Re-include Windows platform files.
        files {
            "Src/Platform/Windows/**.h",
            "Src/Platform/Windows/**.cpp"
        }

        -- Re-include OpenGL (for now).
        files {
            "Src/Platform/Graphics/OpenGL/**.h",
            "Src/Platform/Graphics/OpenGL/**.cpp"
        }

        links {
            "opengl32",
            "glad"
        }

    filter "system:linux"
        defines {
            "AG_PLATFORM_LINUX"
        }

    filter "system:macosx"
        defines {
            "AG_PLATFORM_MAC"
        }

    filter "configurations:Debug"
        runtime "Debug"
        symbols "On"
        targetsuffix "-d"

        defines {
            "AG_DEBUG"
        }

        libdirs {
            "../%{LibsDir.vcpkg_debug}"
        }

        links {}

    filter "configurations:Release"
        runtime "Release"
        optimize "On"

        defines {
            "AG_RELEASE"
        }

        libdirs {
            "../%{LibsDir.vcpkg_release}"
        }
        
        links {}