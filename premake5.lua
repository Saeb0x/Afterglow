workspace "Afterglow"
    architecture "x86_64"
    configurations {
        "Debug",
        "Release"
    }
    startproject "Sandbox"

    VCPKG_INSTALLED = "vcpkg_installed/x64-windows"
    IncludeDir = {}
    LibDir = {}
    IncludeDir["vcpkg"] = VCPKG_INSTALLED .. "/include"
    LibDir["vcpkg_debug"] = VCPKG_INSTALLED .. "/debug/lib"
    LibDir["vcpkg_release"] = VCPKG_INSTALLED .. "/lib"

    group "Afterglow"
        include "Afterglow"
    group ""
    
    group "Examples"
        include "Examples/Sandbox"
    group ""