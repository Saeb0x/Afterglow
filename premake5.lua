workspace "Afterglow"
    architecture "x86_64"
    configurations {
        "Debug",
        "Release"
    }
    startproject "Sandbox"

    local host = os.host()
    local triplet

    if host == "windows" then
        triplet = "x64-windows"
    elseif host == "linux" then
        triplet = "x64-linux"
    elseif host == "macosx" then
        triplet = "x64-osx"
    else
        error("Unsupported platform: " .. host)
    end

    VCPKG_INSTALLED = "vcpkg_installed/" .. triplet
    
    IncludeDir = {}
    IncludeDir["vcpkg"] = VCPKG_INSTALLED .. "/include"
    
    LibsDir = {}
    LibsDir["vcpkg_debug"] = VCPKG_INSTALLED .. "/debug/lib"
    LibsDir["vcpkg_release"] = VCPKG_INSTALLED .. "/lib"

    group "Afterglow"
        include "Afterglow"
    group ""
    
    group "Examples"
        include "Examples/Sandbox"
        include "Examples/Wave"
    group ""