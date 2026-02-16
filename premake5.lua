workspace "Afterglow"
    architecture "x86_64"
    configurations {
        "Debug",
        "Release"
    }
    startproject "Sandbox"

    VCPKG_INSTALLED = "vcpkg_installed/x64-windows"
    IncludeDir = {}
    LibsDir = {}
    IncludeDir["vcpkg"] = VCPKG_INSTALLED .. "/include"
    LibsDir["vcpkg_debug"] = VCPKG_INSTALLED .. "/debug/lib"
    LibsDir["vcpkg_release"] = VCPKG_INSTALLED .. "/lib"

    group "Afterglow"
        include "Afterglow"
    group ""
    
    group "Examples"
        include "Examples/Sandbox"
    group ""