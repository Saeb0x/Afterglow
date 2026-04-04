project "Sandbox"
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
        "../../Afterglow/Src"
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

    filter "configurations:Debug"
        runtime "Debug"
        symbols "On"
        targetsuffix "-d"
        
        defines {
            "AG_DEBUG"
        }

    filter "configurations:Release"
        runtime "Release"
        optimize "On"

        defines {
            "AG_RELEASE"
        }