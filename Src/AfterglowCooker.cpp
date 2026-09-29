// NOTE(saeb): AfterglowCooker as one translation unit (a "unity build"), for the same reason as Afterglow.cpp: a game's build compiles this one file, whatever the cooker is made of.

// Portable cooker.
#include "Cooker/Cooker.cpp"
#include "Cooker/CookTexture.cpp"
#include "Cooker/CookShader.cpp" // The D3D11 shader target
#include "Cooker/CookFont.cpp"

// Platform layer.
#include "Engine/Platform/Windows/Win32File.cpp"
#include "Cooker/Platform/Windows/Win32CookerMain.cpp"
